#ifndef CONFIG_H
#define CONFIG_H

// Define the version macro to choose the pinout configuration
// Uncomment one of the following lines to select the device version
//#define ESP32_WROOM
#define NODEMCU

#ifdef ESP32_WROOM
    #define SD_MISO 2  // D0
    #define SD_MOSI 15 // D3
    #define SD_CLK 14  // SCK
    #define SD_CS 13   // CMD

#elif defined(NODEMCU)
    // VSPI of nodemcu devboard, used for SD communication in devboard sandwich
    #define SD_MISO 19  // D0
    #define SD_MOSI 23 // D3
    #define SD_CLK 18  // SCK
    #define SD_CS 5   // CMD
#else
    #error "Device version not defined. Please define DEVICE_VERSION_1 or DEVICE_VERSION_2."
#endif

/*
Pins in use. The SPI Master can use the GPIO mux, so feel free to change these if needed.
*/
#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2 // the board currently in use for PepperX
    //#define GPIO_HANDSHAKE 2
    // HSPI of nodemcu devboard, used as SPI slave in devboard sandwich
    #define SPI_MOSI 13
    #define SPI_MISO 12
    #define SPI_SCLK 14
    #define SPI_CS 15

#elif CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32C2 || CONFIG_IDF_TARGET_ESP32H2 // other board supported
    //#define GPIO_HANDSHAKE 3
    #define SPI_MOSI 7
    #define SPI_MISO 2
    #define SPI_SCLK 6
    #define SPI_CS 10

#elif CONFIG_IDF_TARGET_ESP32S3 // other board supported
    //#define GPIO_HANDSHAKE 2
    #define SPI_MOSI 11
    #define SPI_MISO 13
    #define SPI_SCLK 12
    #define SPI_CS 10

#endif //CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2

#endif // CONFIG_H