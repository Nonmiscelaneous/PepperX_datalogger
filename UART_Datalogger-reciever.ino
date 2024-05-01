/*
  SD card file dump

  This example shows how to read a file from the SD card using the
  SD library and send it over the serial port.

  The circuit:
   SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4 (for MKRZero SD: SDCARD_SS_PIN)

  created  22 December 2010
  by Limor Fried
  modified 9 Apr 2012
  by Tom Igoe

  This example code is in the public domain.

*/

#include <SPI.h>
#include <SD.h>

const int chipSelect = 4;
const int led_green = 8;
const int led_red = LED_BUILTIN;

#define cardSelect 4

File logfile;


void setup() {
  pinMode(led_red, OUTPUT);
  pinMode(led_green, OUTPUT);

  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("\r\nAnalog logger test");

  Serial1.begin(115200);
  while (!Serial) delay(1);
  while (!Serial1) delay(1); 

  Serial.print("Initializing SD card...");

  // test if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
  Serial.println("card initialized.");

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  logfile = SD.open("test.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (logfile) {
    Serial.print("Writing to test.txt...");
    logfile.println("testing 1, 2, 3.");
    // close the file:
    logfile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }

}

void loop() {
  logfile = SD.open("test.txt", FILE_WRITE);
  while (Serial1.available()){
    digitalWrite(led_green, HIGH);
    int inByte = Serial1.read();
    Serial.write(inByte);
    logfile.write(inByte);
    digitalWrite(led_green, LOW);
  }
  logfile.close(); 
  //Serial.println("log closed");
}

