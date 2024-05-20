#include <SPI.h>
#include "helper.h"

// MOSI pin: gpio 23 (labelled p23)
// MISO pin: gpio 19 (labelled p19)
// SCLK pin: gpio 18 (labelled p18)
// CS pin: gpio 15 (labelled p15)

SPIClass master(HSPI); // call SPI methods with master.<command>
#ifdef CONFIG_IDF_TARGET_ESP32
static constexpr uint8_t PIN_SS = 15; // sets SS pin to gpio 15 (p15 label)
#else
static constexpr uint8_t PIN_SS = SS; // sets SS pin to default, gpio 5 (p5 label)
#endif

static constexpr size_t BUFFER_SIZE = 4; // number of bytes in buffer
uint8_t tx_buf[BUFFER_SIZE] {0}; // initialize send buffer contents
uint8_t rx_buf[BUFFER_SIZE] {0}; // initialize recieve buffer contents
uint32_t ck_speed = 12500000; // SPI clock speed in Hz
//uint16_t data_delay = ; // delay between transmissions in miliseconds
uint8_t i = 0; // initialize index to 0

void setup()
{
    Serial.begin(115200); // starts usb serial to PC with baud of 115200

    delay(2000); // wait 2000 miliseconds to allow serial to initialize

    pinMode(PIN_SS, OUTPUT); // sets SS pin to output
    digitalWrite(PIN_SS, HIGH); // sets SS pin to high
    master.begin(SCK, MISO, MOSI, PIN_SS); // initialize spi master

    delay(2000); // wait 2000 miliseconds to allow spi to initialize

    Serial.println("start spi master"); // prints usb serial monitor
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
    
        //dumpBuffers("tx 1", tx_buf, 0, BUFFER_SIZE); // dumps contents of tx_buf
        //dumpBuffers("rx 1", rx_buf, 0, BUFFER_SIZE); // dumps contents of rx_buf
    master.beginTransaction(SPISettings(ck_speed, MSBFIRST, SPI_MODE0));
    // starts spi transaction with settings for clock speed, bit order, spimode
    // ck_speed: clock speed
    // MSBFORST: most significant bit first
    
    digitalWrite(PIN_SS, LOW); // sets SS to low (selecting the associated slave)
    master.transferBytes(tx_buf, rx_buf, BUFFER_SIZE);
    // sends contents of tx_buf to slave
    // writes data recieved from slave to rx_buf
        //dumpBuffers("tx 2", tx_buf, 0, BUFFER_SIZE); // dumps contents of tx_buf
        //dumpBuffers("rx 2", rx_buf, 0, BUFFER_SIZE); // dumps contents of rx_buf
    digitalWrite(PIN_SS, HIGH);
    master.endTransaction();
        //dumpBuffers("tx 3", tx_buf, 0, BUFFER_SIZE); // dumps contents of tx_buf
        //dumpBuffers("rx 3", rx_buf, 0, BUFFER_SIZE); // dumps contetns of rx_buf

    // verify and dump difference with received data
    /*
    if (verifyAndDumpDifference("master", tx_buf, BUFFER_SIZE, "slave", rx_buf, BUFFER_SIZE)) {
        Serial.println("successfully received expected data from slave");
        dumpBuffers("tx", tx_buf, 0, BUFFER_SIZE); // dumps contents of tx_buf
        //dumpBuffers("rx", rx_buf, 0, BUFFER_SIZE); // dumps contetns fo rx_buf
    } else {
        Serial.println("unexpected difference found between master/slave data");
        dumpBuffers("tx", tx_buf, 0, BUFFER_SIZE); // dumps contents of tx_buf
        //dumpBuffers("rx", rx_buf, 0, BUFFER_SIZE); // dumps contents of rx_buf
    }
    */

    dumpBuffers("tx", tx_buf, 0, BUFFER_SIZE); // dumps contents of tx_buf

    //delay(0.025); // delay between transactions
}
