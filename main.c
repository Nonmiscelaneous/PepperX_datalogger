/*
    SPI slave reciever that writes what it recieves to SDcard
*/

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"
#include "esp_err.h"
#include "driver/spi_slave.h"
#include "driver/gpio.h"

#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

#include "esp_timer.h"
#include <rom/ets_sys.h> //For ets_delay_us()

#include "config.h"


// host definitions
#define SPI_SLAVE_HOST HSPI_HOST
#define SD_SPI_HOST VSPI_HOST

// mounting definitions
#define MOUNT_POINT "/sdcard"
const char *file_path = MOUNT_POINT "/new_data.txt";

// tag for error logging
static const char *TAG = "esp32_sd_spi";

// size definitions
#define WRITE_CHUNK 32768 // Size of data buffer for write operations. In bytes. Max seems t be 32768
#define COMM_SIZE 3 // Size of buffer for storing recieved data. In bytes. Will recieve 3 bytes at a time from FPGA

// queueing
#define QUEUE_LENGTH 1
#define QUEUE_ITEM_SIZE sizeof(data_chunk_t)
static QueueHandle_t data_queue;

//  benchmarking constants
#define MEASURE_INTERVAL_MS 5000 // Measure interval in milliseconds
static uint32_t total_bytes_written = 0;

typedef struct {
    char data[WRITE_CHUNK];
} data_chunk_t;

typedef struct ring_buffer{
    uint32_t head;
    uint32_t buff_size;
    char contents[WRITE_CHUNK];
} ring_buffer;

// function to initialize buffer
/*
void ring_init(ring_buffer *ringbuf, size_t capacity, size_t sz){

}
*/

void ringbuf_write(ring_buffer *ringbuf, char *data, int num_samples){
    int buf_len = ringbuf->buff_size - 1;
    int last_written_sample = ringbuf->head;

    int j;
    for (int i = 0; i < num_samples;i++){
        j = (i + last_written_sample) & buf_len;
        ringbuf->contents[j] = data[i];
    }

    // update head location
    ringbuf->head += num_samples;
    ringbuf->head &= ringbuf->buff_size - 1;
}

// task for listening to SPI master
static void
spi_slave_task()
{
    ESP_LOGI(TAG, "Starting spi_slave_task of spi reciever to sdcard...\n");
    int i = 0;
    esp_err_t ret_spi;
    // qdata_chunk_t chunk;
    //  ??? replace recvbuf with ringbuffer ???
    //  can set buffer size any interger [1, 32] if DMA not enabled (this code doesn't use DMA)
    WORD_ALIGNED_ATTR char sendbuf[COMM_SIZE] = ""; // allocates for the send buffer, need to figure what type to use, char seems to work well enough
    WORD_ALIGNED_ATTR char recvbuf[COMM_SIZE] = ""; // allocates for the recieve buffer, need to figure what type to use
    memset(recvbuf, 0, COMM_SIZE); // fills recvbuf with 0's
    spi_slave_transaction_t t; // t is transaction struct
    memset(&t, 0, sizeof(t)); // sets all bytes of t to 0

    // initialize ring buffer

    ring_buffer *recieved_buffer = (ring_buffer *)calloc(1, sizeof(ring_buffer));
    recieved_buffer->buff_size = WRITE_CHUNK;
    recieved_buffer->head = 0;

    while(1) {
        //ESP_LOGI(TAG, "Start of while loop...\n");
        //Clear receive buffer, set send buffer to something sane
        memset(recvbuf, 0x00, COMM_SIZE); // first COMM_SIZE bytes of recvbuf to the value of second arg
        memset(sendbuf, 0xAA, COMM_SIZE); // first COMM_SIZE bytes of sendbuf to the value of second arg

        //Set up a transaction of 4 bytes to send/receive
        t.length = COMM_SIZE*8; // ? sets length of t ?
        t.tx_buffer = NULL; // transfers of contents between tx_buffer in t (transfer) struct and the sendbuf (send buffer)
        t.rx_buffer = recvbuf; // transfers of contents between rx_buffer in t (transfer) struct and the recvbuf (recieve buffer)
        //ret_spi = spi_slave_transmit(SPI_SLAVE_HOST, &t, portMAX_DELAY); // one possible way to implement spi transactions
        spi_slave_transmit(SPI_SLAVE_HOST, &t, portMAX_DELAY);
        i++;
        //ESP_LOGI(TAG, "after transmittion... %d", i);

        //spi_slave_transmit does not return until the master has done a transmission, so by here we have received data from the master.
        // todo: insert recieve buffer transfering here
        ringbuf_write(recieved_buffer, recvbuf, COMM_SIZE);

        // TODO: some condition so that recieved_buffer is sent to queue every loop, only when it cycles through so much data
        // options: 
        // 1: counter, since each data packet should be 3 bytes: so after X recieves, send to queue
            // 10922 transactions will fill buffer 32766, add 2 zeros at the end? or leave a "doubled up" set of data points?
        // 2:
        if (i == 10922) { 
            // Place data chunk onto the queue
            if (xQueueSend(data_queue, &recieved_buffer, portMAX_DELAY) != pdPASS){
                ESP_LOGE(TAG, "Failed to send data to queue");
            }
            else{
                ESP_LOGI(TAG, "Data recieved: %d", sizeof(recieved_buffer));
            }

            i = 0;
        }
        // ets_delay_us(DATA_GEN_DELAY_US); // busy wait delay here in micro seconds instead of millisecond non-blocking delay to simulate data input read
    }

}

static int write_data_to_file(const char *path, char *data)
{
    int written_bytes = 0;
    // Open the file
    FILE *file = fopen(path, "a");
    if (file == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for appending");
        return ESP_FAIL;
    }
    // Set to NULL buffer (dynamically determined buffer size), Full Buffering, with larger stream buffer size
    setvbuf(file, NULL, _IOFBF, 32 * 1024);

    // Write the data
    //written_bytes = fprintf(file, data);
    written_bytes = fwrite(data, sizeof(char), WRITE_CHUNK, file);
    ESP_LOGI(TAG, "number of bytes written: %d", written_bytes);
    if (written_bytes < 0)
    {
        ESP_LOGE(TAG, "Failed to write data to the file");
    }
    // TODO - try writing data using fwrite as we do not need formatting provided by fprintf with the raw byte data

    fclose(file);

    return written_bytes;
}

// task for writing recieved data to sdcard
static void sd_card_writer_task(){

    data_chunk_t chunk;
    ESP_LOGI(TAG, "Starting sd_writer_task of spi reciever to sdcard...\n");

    int64_t start_time_us = esp_timer_get_time();
    int64_t elapsed_time_us = 0;

    while(1){
        // wait for data to arrive on the queue
        if (xQueueReceive(data_queue, &chunk, portMAX_DELAY) == pdPASS){
            total_bytes_written += (uint32_t) write_data_to_file(file_path, &chunk.data);
            ESP_LOGI(TAG, "Contents of recieved Queue: %s", chunk.data);
        }
        elapsed_time_us = esp_timer_get_time() - start_time_us;

        if(elapsed_time_us >= MEASURE_INTERVAL_MS * 1000){
            float write_speed_Bps = (total_bytes_written) / (elapsed_time_us / 1000000.0); // in bits per second
            // float write_speed_mbps / 1000000.0; // convert to Mbps
            //ESP_LOGI(TAG, "Write Speed: %.5f Bytes per second ", write_speed_Bps);

            // Reset counters
            total_bytes_written = 0;
            start_time_us = esp_timer_get_time();
        }
    }
}

void app_main(){
    ESP_LOGI(TAG, "Starting app_main of spi reciever to sdcard...\n");

    esp_err_t ret_spi;
    esp_err_t ret_sd;

// SD card configurations
    esp_vfs_fat_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 6,
        .allocation_unit_size = 64 * 1024};

    sdmmc_card_t *card;
    const char mount_point[] = MOUNT_POINT;
    sdmmc_host_t host = SDSPI_HOST_DEFAULT(); // default 20MHz, HSPI
    host.slot = SD_SPI_HOST; // hopefully changes to VSPI

    spi_bus_config_t sdbus_cfg = {
        .mosi_io_num = SD_MOSI,
        .miso_io_num = SD_MISO,
        .sclk_io_num = SD_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4092,
    };

    // SD and bus initialization
    ret_sd = spi_bus_initialize(host.slot, &sdbus_cfg, SD_SPI_HOST);
    if (ret_sd != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to init the spi bus\n");
        return;
    }

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = SD_CS;
    slot_config.host_id = host.slot;

    ESP_LOGI(TAG, "Mounting filesystem");
    ret_sd = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret_sd != ESP_OK)
    {
        if (ret_sd == ESP_FAIL)
        {
            ESP_LOGE(TAG, "Failed to mount filesystem. ");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                          "Make sure SD card lines have pull-up resistors in place.",
                     esp_err_to_name(ret_sd));
        }
        return;
    }

    ESP_LOGI(TAG, "Filesystem mounted");

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);

// SPI Slave configurations
    spi_bus_config_t spibus_cfg = {
        .mosi_io_num = SPI_MOSI, // sets MOSI pin
        .miso_io_num = SPI_MISO, // sets MISO pin
        .sclk_io_num = SPI_SCLK, // sets CLK pin
        .quadwp_io_num = -1,     // write protect, -1 means not used
        .quadhd_io_num = -1,     // hold signal, -1 means not used
    };

    //Configuration for the SPI slave interface
    spi_slave_interface_config_t slv_cfg = {
        .mode = 0,               // sets spio mode; 0,1,2,3
        .spics_io_num = SPI_CS, // sets chip select
        // queue_size: transaction queue size.
        // This sets how many transactions can be 'in the air'
        //(queued using spi_slave_queue_trans but not yet finished using spi_slave_get_trans_result) at the same time
        .queue_size = 3,
        .flags = 0, // ? spi flags ?
    };

    //Enable pull-ups on SPI lines so we don't detect rogue pulses when no master is connected.
    gpio_set_pull_mode(SPI_MOSI, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(SPI_SCLK, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(SPI_CS, GPIO_PULLUP_ONLY);

    //Initialize SPI slave interface
    ret_spi = spi_slave_initialize(SPI_SLAVE_HOST, &spibus_cfg, &slv_cfg, SPI_DMA_CH_AUTO);
    assert(ret_spi == ESP_OK);

    // Now use POSIX and C standard library functions to work with files thanks to VFS FAT FS mounting.

    // TODO - Create file, write to data for specific duration of time

    // Create a FreeRTOS queue for byte data
    data_queue = xQueueCreate(QUEUE_LENGTH, QUEUE_ITEM_SIZE);
    if (data_queue == NULL)
    {
        ESP_LOGE(TAG, "Failed to create queue");
        return;
    }

    // TODO - check if data file exists already - if yes then wipe it , otherwise ignore (write fxn opens for appending)
    ESP_LOGI(TAG, "Just before tasks are pinned to cores");

    xTaskCreatePinnedToCore(sd_card_writer_task, "sd_card_writer_task", 77777, NULL, 7, NULL, 1); // Run on core 0
    xTaskCreatePinnedToCore(spi_slave_task, "spi_slave_task", 77777, NULL, 5, NULL, 0); // Run on core 1

}