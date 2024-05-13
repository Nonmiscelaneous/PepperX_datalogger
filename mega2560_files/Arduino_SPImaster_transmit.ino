#include <SPI.h>
uint8_t val;
void setup(){
  Serial.begin(115200);

  SPI.begin();
  digitalWrite(SS, HIGH);
  SPI.beginTransaction(SPISettings(200000, MSBFIRST, SPI_MODE0));
  //SPI.setClockDivider(SPI_CLOCK_DIV64);
}

void loop(){
  for (int i = 1; i <= 0xff; i++) {
  digitalWrite(SS,LOW);
  int r = SPI.transfer(i);
  digitalWrite(SS,HIGH);
  Serial.println(r);
  delay(10000);
  }
}
