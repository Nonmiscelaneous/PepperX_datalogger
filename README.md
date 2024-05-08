# PepperX_FeatherM0_datalogger

Tutorial to setup the Feather to work with Arduino IDE:
https://learn.adafruit.com/adafruit-feather-m0-adalogger/setup

The objective: Operate the Feather as an SPI slave to recieve data (in HEX) and write that data to SD card as it is recieved.

The Feather is to slow for the intended purpose, so swithching to ESP32

ESP 32 references:
- https://randomnerdtutorials.com/esp32-spi-communication-arduino/#spi-multiple-bus-hspi-vspi
- https://randomnerdtutorials.com/esp32-microsd-card-arduino/#writeFile
- https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/spi_slave.html
- https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/spi_slave.html#spi-dma-known-issues
- https://microcontrollerslab.com/esp32-spi-communication-tutorial-arduino/
- https://github.com/hideakitai/ESP32SPISlave/tree/main
- https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf
- https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf#sdioslave
