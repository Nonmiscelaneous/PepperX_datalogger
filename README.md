# PepperX_datalogger

## ESP32 is contending microcontroller for this purpose
The Arduino framework code is modified from examples in the https://github.com/hideakitai/ESP32SPISlave/tree/main repository. 
The IDF framework code is modified from examples in the https://github.com/espressif/esp-idf repository.

Currently used is Master_transmitter_ArduinoFramework as test sender and Slave_Reciever_ESP-IDFframework or Slave_Reciever_ArduinoFramework as the recievers. (Note: folder structure is from PlatfromIO, main is in 'src' and headerfiles are in 'include'

To install in Arduino IDE, in the bar at the top select: Tools -> Manage Libraries, in the search bar type "esp32 spi slave" and the library will come up, click install.

The files being worked on are in working_files, note that ES#@SPISlave.h is a copy of the file https://github.com/hideakitai/ESP32SPISlave/blob/main/ESP32SPISlave.h

## ESP 32 references:
![esp32_pinout](https://m.media-amazon.com/images/I/81qFXiQh-ZL._AC_SL1500_.jpg)
- https://randomnerdtutorials.com/esp32-spi-communication-arduino/#spi-multiple-bus-hspi-vspi
- https://randomnerdtutorials.com/esp32-microsd-card-arduino/#writeFile
- https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/spi_slave.html
- https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/spi_slave.html#spi-dma-known-issues
- https://microcontrollerslab.com/esp32-spi-communication-tutorial-arduino/
- https://github.com/hideakitai/ESP32SPISlave/tree/main
- https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf
- https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf#sdioslave

Feather was previous microcontroller, no longer used due to being to slow.
