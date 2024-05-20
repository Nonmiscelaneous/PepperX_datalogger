/* SPI Slave example, receiver (uses SPI Slave driver to communicate with sender)

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "driver/spi_slave.h"
#include "driver/gpio.h"


/*
SPI receiver (slave) example.

This example is supposed to work together with the SPI sender. It uses the standard SPI pins (MISO, MOSI, SCLK, CS) to
transmit data over in a full-duplex fashion, that is, while the master puts data on the MOSI pin, the slave puts its own
data on the MISO pin.

This example uses one extra pin: GPIO_HANDSHAKE is used as a handshake pin. After a transmission has been set up and we're
ready to send/receive data, this code uses a callback to set the handshake pin high. The sender will detect this and start
sending a transaction. As soon as the transaction is done, the line gets set low again.
*/

/*
Pins in use. The SPI Master can use the GPIO mux, so feel free to change these if needed.
*/
#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2 // the board currently in use for PepperX
//#define GPIO_HANDSHAKE 2
#define GPIO_MOSI 13
#define GPIO_MISO 12
#define GPIO_SCLK 14
#define GPIO_CS 15

#elif CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32C2 || CONFIG_IDF_TARGET_ESP32H2 // other board supported
//#define GPIO_HANDSHAKE 3
#define GPIO_MOSI 7
#define GPIO_MISO 2
#define GPIO_SCLK 6
#define GPIO_CS 10

#elif CONFIG_IDF_TARGET_ESP32S3 // other board supported
//#define GPIO_HANDSHAKE 2
#define GPIO_MOSI 11
#define GPIO_MISO 13
#define GPIO_SCLK 12
#define GPIO_CS 10

#endif //CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2


#ifdef CONFIG_IDF_TARGET_ESP32
#define RCV_HOST    HSPI_HOST

#else
#define RCV_HOST    SPI2_HOST

#endif


/* hand shake shenanigans, don't want *
//Called after a transaction is queued and ready for pickup by master. We use this to set the handshake line high.
void my_post_setup_cb(spi_slave_transaction_t *trans) {
    gpio_set_level(GPIO_HANDSHAKE, 1);
}

//Called after transaction is sent/received. We use this to set the handshake line low.
void my_post_trans_cb(spi_slave_transaction_t *trans) {
    gpio_set_level(GPIO_HANDSHAKE, 0);
}
*/

//Main application
void app_main(void)
{
    int n=0; // declare index for counter
    esp_err_t ret; // ? error return declaration ?

    //Configuration for the SPI bus
    spi_bus_config_t buscfg={
        .mosi_io_num=GPIO_MOSI, // sets MOSI pin
        .miso_io_num=GPIO_MISO, // sets MISO pin
        .sclk_io_num=GPIO_SCLK, // sets CLK pin
        .quadwp_io_num = -1,  // write protect, -1 means not used
        .quadhd_io_num = -1,  // hold signal, -1 means not used
    };

    //Configuration for the SPI slave interface
    spi_slave_interface_config_t slvcfg = {
        .mode = 0,               // sets spio mode; 0,1,2,3
        .spics_io_num = GPIO_CS, // sets chip select
        // queue_size: transaction queue size.
        // This sets how many transactions can be 'in the air'
        //(queued using spi_slave_queue_trans but not yet finished using spi_slave_get_trans_result) at the same time
        .queue_size = 3,
        .flags = 0, // ? spi flags ?
        /* hand shake shenanigans, don't want *
        .post_setup_cb=my_post_setup_cb,
        .post_trans_cb=my_post_trans_cb
        */
    };

    /* hand shake shenanigans, don't want *
    //Configuration for the handshake line
    gpio_config_t io_conf={
        .intr_type=GPIO_INTR_DISABLE,
        .mode=GPIO_MODE_OUTPUT,
        .pin_bit_mask=(1<<GPIO_HANDSHAKE)
    };
    */

    /* hand shake shenanigans, don't want *
    //Configure handshake line as output
    gpio_config(&io_conf);
    */
    //Enable pull-ups on SPI lines so we don't detect rogue pulses when no master is connected.
    gpio_set_pull_mode(GPIO_MOSI, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(GPIO_SCLK, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(GPIO_CS, GPIO_PULLUP_ONLY);

    //Initialize SPI slave interface
    ret=spi_slave_initialize(RCV_HOST, &buscfg, &slvcfg, SPI_DMA_CH_AUTO);
    assert(ret==ESP_OK);

    // can set buffer size any interger [1, 32] if DMA not enabled (this code doesn't use DMA)
    WORD_ALIGNED_ATTR char sendbuf[4]=""; // allocates for the send buffer, need to figure what type to use
    WORD_ALIGNED_ATTR char recvbuf[4]=""; // allocates for the recieve buffer, need to figure what type to use
    memset(recvbuf, 0, 4); // fills recvbuf with 0's
    spi_slave_transaction_t t; // t is transaction struct
    memset(&t, 0, sizeof(t)); // sets all bytes of t to 0

    while(1) {
        //Clear receive buffer, set send buffer to something sane
        memset(recvbuf, 0xA5, 4); // first 4 bytes of recvbuf to the value of second arg
        memset(sendbuf, 0xAA, 4); // first 4 bytes of sendbuf to the value of second arg

        //Set up a transaction of 4 bytes to send/receive
        t.length=4*8; // ? sets length of t ?
        t.tx_buffer=sendbuf; // transfers of contents between tx_buffer in t (transfer) struct and the sendbuf (send buffer)
        t.rx_buffer=recvbuf; // transfers of contents between rx_buffer in t (transfer) struct and the recvbuf (recieve buffer)
        /* This call enables the SPI slave interface to send/receive to the sendbuf and recvbuf. The transaction is
        initialized by the SPI master, however, so it will not actually happen until the master starts a hardware transaction
        by pulling CS low and pulsing the clock etc. NOTE: hand shake removed, In this specific example, we use the handshake line, pulled up by the
        .post_setup_cb callback that is called as soon as a transaction is ready, to let the master know it is free to transfer
        data.
        */
        ret=spi_slave_transmit(RCV_HOST, &t, portMAX_DELAY); // one possible way to implement spi transactions

        //spi_slave_transmit does not return until the master has done a transmission, so by here we have sent our data and
        //received data from the master. Print it.
        printf("%s", recvbuf);
        n++;
    }

}