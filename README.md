# PepperX_datalogger
## Repository Structure
### esp32_fpga_standin 
Follows the PlatformIO build chain and file structure. the code files to extract for your own build chain are in 'src/' (the main file of the program) and the header files in 'include'.
This program acts as a SPI Master and transmits 24 bit 'packets' (as 3 char in c) at a set interval. (Currently 1 ms delay so USB serial can keep up with transmissions)

### esp32_spi_slave_recieve_to_sdcard 
Follows the PlatformIO build chain and file structure. the code files to extract for your own build chain are in 'src/' (the main file of the program) and the header files in 'include'.
This program recieves data as an SPI slave. Has 3 byte (char) recieve buffer that passes to a larger buffer for dumping 32 768 bytes to SD card. 


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
