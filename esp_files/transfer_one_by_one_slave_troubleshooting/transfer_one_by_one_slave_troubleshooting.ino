#include <ESP32SPISlave.h>
#include "helper.h"

// MOSI pin: gpio 13 (labelled p13)
// MISO pin: gpio 12 (labelled p12)
// SCLK pin: gpio 14 (labelled p14)
// CS pin: gpio 15 (labelled p15)

// Allows calling of ESP32SPISlaves.h methods using "slave.<command>"
ESP32SPISlave slave;

// NOTE: Buffer size is designed to be multiples of 4
static constexpr size_t BUFFER_SIZE = 4; // number of bytes in buffer
static constexpr size_t QUEUE_SIZE = 1; // ??? number of buffers to wait for ???
uint8_t tx_buf[BUFFER_SIZE] {0}; // transmit message initialization
// since this is the slave code, tx_buf is sent to the master over the MISO line
uint8_t rx_buf[BUFFER_SIZE] {0}; // initialization of recieved message buffer
// initialized to 0's, this will be over-written by the message recieved from the master

void setup()
{
    Serial.begin(921600); // Serial connection to PC over USB
    // NOTE: 1 000 000 and 2 000 000 gave garbled messages in the serial monitor

    delay(2000); // wait 2000 miliseconds for communication to be initialized and opened

    slave.setDataMode(SPI_MODE0);   // default: SPI_MODE0
    // sets spi mode, 0,1,2,3 are all supported
    
    slave.setQueueSize(QUEUE_SIZE); // default: 1
    // sets queue size, so far only know something to do with freeRTOS apparently?
    // there is a queue.h in arduino ide that is called upon and references freeRTOS

    // begin() after setting
    slave.begin();  // default: HSPI (please refer README for pin assignments)
    // can select the bus; HSPI, VSPI, FSPI
    // can select pins with the order; sck, miso, mosi, ss
    // example: begin(HSPI, 1,2,3,4) NOTE this is unverified, mearly prediction

    Serial.println("start spi slave"); // writes message to pc serial monitor

// end of setup
}

void loop()
{
    // initialize tx/rx buffers
    // this initialialization involves rewriting the buffers with a loop that counts
    // from an offset (default 0) 
    //initializeBuffers(tx_buf, rx_buf, BUFFER_SIZE);
    // ultimately, a reply to the master is not needed and unused so it can stay as 0's
    // for the sake of speed, just overwrite the previous values of rx_buf without
    // resetting everything to 0

    // start and wait to complete one BIG transaction
    const size_t received_bytes = slave.transfer(tx_buf, rx_buf, BUFFER_SIZE);
    // This is the function that handles the recieving of data
    // 1st argument: transmit buffer
    // 2nd argument: recieve buffer
    // 3rd argument: number of bytes a buffer (tx and rx are the same size)

    // verifyAndDumpDifferences: compares tx_buf to rx_buf and prints differences
    /*
    if (verifyAndDumpDifference("slave", tx_buf, BUFFER_SIZE, "master", rx_buf, received_bytes)) {
        Serial.println("successfully received expected data from master");
        //dumpBuffers("tx", tx_buf, 0,sizeof(tx_buf)); // prints tx_buf to monitor
        dumpBuffers("rx", rx_buf, 0,sizeof(rx_buf)); // prints rx_buf to monitor
    } else {
        Serial.println("unexpected difference found between master/slave data");
        //dumpBuffers("tx", tx_buf, 0,sizeof(tx_buf)); // prints tx_buf to monitor
        dumpBuffers("rx", rx_buf, 0,sizeof(rx_buf)); // prints rx_buf to monitor
    }
    */
    
    dumpBuffers("rx", rx_buf, 0,sizeof(rx_buf)); // prints rx_buf to monitor

}
