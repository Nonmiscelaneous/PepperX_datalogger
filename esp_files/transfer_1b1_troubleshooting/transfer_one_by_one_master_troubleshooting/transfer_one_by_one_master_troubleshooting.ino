#include <SPI.h>
#include "helper.h"

SPIClass master(HSPI);
#ifdef CONFIG_IDF_TARGET_ESP32
static constexpr uint8_t PIN_SS = 15;
#else
static constexpr uint8_t PIN_SS = SS;
#endif

static constexpr size_t BUFFER_SIZE = 1;
uint8_t tx_buf[BUFFER_SIZE] {0xAA};
uint8_t rx_buf[BUFFER_SIZE] {0};

void setup()
{
    Serial.begin(115200);

    delay(2000);

    pinMode(PIN_SS, OUTPUT);
    digitalWrite(PIN_SS, HIGH);
    master.begin(SCK, MISO, MOSI, PIN_SS);

    delay(2000);

    Serial.println("start spi master");
}

void loop()
{
    // initialize tx/rx buffers
    //initializeBuffers(tx_buf, rx_buf, BUFFER_SIZE);
        dumpBuffers("tx 1", tx_buf, 0, BUFFER_SIZE);
        dumpBuffers("rx 1", rx_buf, 0, BUFFER_SIZE);
    master.beginTransaction(SPISettings(10000, MSBFIRST, SPI_MODE0));
    digitalWrite(PIN_SS, LOW);
    master.transferBytes(tx_buf, rx_buf, BUFFER_SIZE);
        dumpBuffers("tx 2", tx_buf, 0, BUFFER_SIZE);
        dumpBuffers("rx 2", rx_buf, 0, BUFFER_SIZE);
    digitalWrite(PIN_SS, HIGH);
    master.endTransaction();
        dumpBuffers("tx 3", tx_buf, 0, BUFFER_SIZE);
        dumpBuffers("rx 3", rx_buf, 0, BUFFER_SIZE);

    // verify and dump difference with received data
    if (verifyAndDumpDifference("master", tx_buf, BUFFER_SIZE, "slave", rx_buf, BUFFER_SIZE)) {
        Serial.println("successfully received expected data from slave");
        dumpBuffers("tx", tx_buf, 0, BUFFER_SIZE);
        dumpBuffers("rx", rx_buf, 0, BUFFER_SIZE);
    } else {
        Serial.println("unexpected difference found between master/slave data");
        dumpBuffers("tx", tx_buf, 0, BUFFER_SIZE);
        dumpBuffers("rx", rx_buf, 0, BUFFER_SIZE);
    }

    delay(2000);
}