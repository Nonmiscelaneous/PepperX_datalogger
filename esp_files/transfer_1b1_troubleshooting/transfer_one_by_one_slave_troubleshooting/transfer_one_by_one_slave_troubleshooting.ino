#include <ESP32SPISlave.h>
#include "helper.h"

// MOSI 13
// MISO 12
// SCLK 14
// CS 15

ESP32SPISlave slave;

static constexpr size_t BUFFER_SIZE = 4;
static constexpr size_t QUEUE_SIZE = 1;
uint8_t tx_buf[BUFFER_SIZE] {170, 170,170,170};
uint8_t rx_buf[BUFFER_SIZE] {0, 0, 0 ,0};

void setup()
{
    Serial.begin(921600);

    delay(2000);

    slave.setDataMode(SPI_MODE0);   // default: SPI_MODE0
    slave.setQueueSize(QUEUE_SIZE); // default: 1

    // begin() after setting
    slave.begin();  // default: HSPI (please refer README for pin assignments)

    Serial.println("start spi slave");
    //MOSI
    //MISO
    //SCK
    //SS
}

void loop()
{
    // initialize tx/rx buffers
    //initializeBuffers(tx_buf, rx_buf, BUFFER_SIZE);

    // start and wait to complete one BIG transaction (same data will be received from slave)
    const size_t received_bytes = slave.transfer(tx_buf, rx_buf, BUFFER_SIZE);

    // verify and dump difference with received data
    if (verifyAndDumpDifference("slave", tx_buf, BUFFER_SIZE, "master", rx_buf, received_bytes)) {
        Serial.println("successfully received expected data from master");
        dumpBuffers("tx", tx_buf, 0,sizeof(tx_buf));
        dumpBuffers("rx", rx_buf, 0,sizeof(rx_buf));
    } else {
        Serial.println("unexpected difference found between master/slave data");
        dumpBuffers("tx", tx_buf, 0,sizeof(tx_buf));
        dumpBuffers("rx", rx_buf, 0,sizeof(rx_buf));
    }
}
