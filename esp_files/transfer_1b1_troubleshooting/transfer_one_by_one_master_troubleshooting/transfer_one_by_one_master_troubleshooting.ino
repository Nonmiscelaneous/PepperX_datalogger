#include <SPI.h>
#include "helper.h"

SPIClass master(HSPI);
#ifdef CONFIG_IDF_TARGET_ESP32
static constexpr uint8_t PIN_SS = 15;
#else
static constexpr uint8_t PIN_SS = SS;
#endif

static constexpr size_t BUFFER_SIZE = 4;
uint8_t tx_buf[BUFFER_SIZE] {0xAA, 0xAA, 0xAA, 0xAA};
uint8_t rx_buf[BUFFER_SIZE] {0, 0, 0, 0};
uint32_t ck_speed = 100000;
uint16_t data_delay = 2;
uint8_t i = 0;

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
    // There is an offset set to 0 int initialize buffers, when testing try:
    initializeBuffers(tx_buf, rx_buf, BUFFER_SIZE, i);
    // This should result in tx containing 0,1,2,3 (for a size of 4, goes up with larger buffer)
    // and with each loop the starting digit will increase by 1.
    //tx_buf[0] = i;
    i++;
    
        //dumpBuffers("tx 1", tx_buf, 0, BUFFER_SIZE);
        //dumpBuffers("rx 1", rx_buf, 0, BUFFER_SIZE);
    master.beginTransaction(SPISettings(ck_speed, MSBFIRST, SPI_MODE0));
    digitalWrite(PIN_SS, LOW);
    master.transferBytes(tx_buf, rx_buf, BUFFER_SIZE);
        //dumpBuffers("tx 2", tx_buf, 0, BUFFER_SIZE);
        //dumpBuffers("rx 2", rx_buf, 0, BUFFER_SIZE);
    digitalWrite(PIN_SS, HIGH);
    master.endTransaction();
        //dumpBuffers("tx 3", tx_buf, 0, BUFFER_SIZE);
        //dumpBuffers("rx 3", rx_buf, 0, BUFFER_SIZE);

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

    delay(data_delay);
}
