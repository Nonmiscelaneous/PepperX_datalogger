#include <SPI.h>
#include "helper.h"

SPIClass master(HSPI);
#ifdef CONFIG_IDF_TARGET_ESP32
static constexpr uint8_t PIN_SS = 15;
#else
static constexpr uint8_t PIN_SS = SS;
#endif

static constexpr size_t BUFFER_SIZE = 8;
uint8_t tx_buf[BUFFER_SIZE] {1, 2, 3, 4, 5, 6, 7, 8};
uint8_t rx_buf[BUFFER_SIZE] {0, 0, 0, 0, 0, 0, 0, 0};

void setup()
{
    Serial.begin(115200);
    Serial.print(tx_buf[1]);
    //Serial.print(rx_buf[1]);

    delay(2000);

    pinMode(PIN_SS, OUTPUT);
    digitalWrite(PIN_SS, HIGH);
    master.begin(SCK, MISO, MOSI, PIN_SS);

    delay(2000);

    Serial.println("start spi master");
}

void loop()
{
    Serial.println(tx_buf[1]);
    // initialize tx/rx buffers
    initializeBuffers(tx_buf, rx_buf, BUFFER_SIZE);
        Serial.println(tx_buf[1]);
        //Serial.print(rx_buf[1]);

    // in this example, the master sends some data first,
    master.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
    digitalWrite(PIN_SS, LOW);
    master.transferBytes(tx_buf, NULL, BUFFER_SIZE);
    digitalWrite(PIN_SS, HIGH);
    master.endTransaction();

    // and the slave sends same data after that
    master.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
    digitalWrite(PIN_SS, LOW);
    master.transferBytes(NULL, rx_buf, BUFFER_SIZE);
    digitalWrite(PIN_SS, HIGH);
    master.endTransaction();

    // verify and dump difference with received data
    if (verifyAndDumpDifference("master", tx_buf, BUFFER_SIZE, "slave", rx_buf, BUFFER_SIZE)) {
        Serial.println("successfully received expected data from slave");
    } else {
        Serial.println("unexpected difference found between master/slave data");
    }

    delay(10000);
}