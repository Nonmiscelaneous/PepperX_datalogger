#include <SPI.h>
#include <SD.h>
#define cardSelect 4

const int slaveAPin = 10;

File logfile;


void setup() {

  Serial.begin(115200);
  while (!Serial) {}

  pinMode(slaveAPin, INPUT_PULLUP);

  attachInterrupt(10, SERCOM1_Handler, FALLING);

  spiSlave_init();

  Serial.println("Test");
  Serial.println("Analog logger test");

  // see if the card is present and can be initialized:
  if (!SD.begin(cardSelect)) {
    Serial.println("Card init. failed!");
  }
  
  Serial.println("SD card OK");
  File root = SD.open("/");
  
  char filename[15];
  strcpy(filename, "/ANALOG00.TXT");
  for (uint8_t i = 0; i < 100; i++) {
    filename[7] = '0' + i/10;
    filename[8] = '0' + i%10;
    // create if does not exist, do not open existing, write, sync after write
    if (! SD.exists(filename)) {
      break;
    }
  }

  logfile = SD.open(filename, FILE_WRITE);
  if( ! logfile ) {
    Serial.print("Couldnt create "); 
    Serial.println(filename);
  }
  Serial.print("Writing to "); 
  Serial.println(filename);
  Serial.println("Ready!");
  logfile.println("TestSDCARD");
  logfile.flush();

}

void loop() {
  //waste some time in a loop
  int var = 0;
  while(var < 200){
  var++;
  }
  logfile.println(var);
  logfile.flush();
  while(1);
}

void spiSlave_init()
{
  
  //Configure SERCOM1 SPI PINS
  //PORTA.DIR.reg &= ~PORT_PA16; //Set PA16 as input (MOSI)
  //PORTA.DIR.reg &= ~PORT_PA17; //Set PA17 as input (SCK)
  //PORTA.DIR.reg &= ~PORT_PA18; //Set PA18 as input (SS)
  //PORTA.DIR.reg |= PORT_PA19; //Set PA19 as output (MISO)

  
  PORT->Group[PORTA].PINCFG[16].bit.PMUXEN = 0x1; //Enable Peripheral Multiplexing for SERCOM1 SPI PA18 Arduino PIN10
  PORT->Group[PORTA].PMUX[8].bit.PMUXE = 0x2; //SERCOM 1 is selected for peripherial use of this pad
  PORT->Group[PORTA].PINCFG[17].bit.PMUXEN = 0x1; //Enable Peripheral Multiplexing for SERCOM1 SPI PA18 Arduino PIN10
  PORT->Group[PORTA].PMUX[8].bit.PMUXO = 0x2; //SERCOM 1 is selected for peripherial use of this pad
  PORT->Group[PORTA].PINCFG[18].bit.PMUXEN = 0x1; //Enable Peripheral Multiplexing for SERCOM1 SPI PA18 Arduino PIN10
  PORT->Group[PORTA].PMUX[9].bit.PMUXE = 0x2; //SERCOM 1 is selected for peripherial use of this pad
  PORT->Group[PORTA].PINCFG[19].bit.PMUXEN = 0x1; //Enable Peripheral Multiplexing for SERCOM1 SPI PA18 Arduino PIN10
  PORT->Group[PORTA].PMUX[9].bit.PMUXO = 0x2; //SERCOM 1 is selected for peripherial use of this pad
  
  //Disable SPI 1
  SERCOM1->SPI.CTRLA.bit.ENABLE =0;
  while(SERCOM1->SPI.SYNCBUSY.bit.ENABLE);
  
  //Reset SPI 1
  SERCOM1->SPI.CTRLA.bit.SWRST = 1;
  while(SERCOM1->SPI.CTRLA.bit.SWRST || SERCOM1->SPI.SYNCBUSY.bit.SWRST);
  
  //Setting up NVIC
  NVIC_EnableIRQ(SERCOM1_IRQn);
  NVIC_SetPriority(SERCOM1_IRQn,2);
  
  //Setting Generic Clock Controller!!!!
  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(GCM_SERCOM1_CORE) | //Generic Clock 0
            GCLK_CLKCTRL_GEN_GCLK0 | // Generic Clock Generator 0 is the source
            GCLK_CLKCTRL_CLKEN; // Enable Generic Clock Generator
  
  while(GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY); //Wait for synchronisation
  
  
  //Set up SPI Control A Register
  SERCOM1->SPI.CTRLA.bit.DORD = 0; //MSB first
  SERCOM1->SPI.CTRLA.bit.CPOL = 0; //SCK is low when idle, leading edge is rising edge
  SERCOM1->SPI.CTRLA.bit.CPHA = 0; //data sampled on leading sck edge and changed on a trailing sck edge
  SERCOM1->SPI.CTRLA.bit.FORM = 0x0; //Frame format = SPI
  SERCOM1->SPI.CTRLA.bit.DIPO = 0; //DATA PAD0 MOSI is used as input (slave mode)
  SERCOM1->SPI.CTRLA.bit.DOPO = 0x2; //DATA PAD3 MISO is used as output
  SERCOM1->SPI.CTRLA.bit.MODE = 0x2; //SPI in Slave mode
  SERCOM1->SPI.CTRLA.bit.IBON = 0x1; //Buffer Overflow notification
  SERCOM1->SPI.CTRLA.bit.RUNSTDBY = 1; //wake on receiver complete
  
  //Set up SPI control B register
  //SERCOM1->SPI.CTRLB.bit.RXEN = 0x1; //Enable Receiver
  SERCOM1->SPI.CTRLB.bit.SSDE = 0x1; //Slave Selecte Detection Enabled
  SERCOM1->SPI.CTRLB.bit.CHSIZE = 0; //character size 8 Bit
  //SERCOM1->SPI.CTRLB.bit.PLOADEN = 0x1; //Enable Preload Data Register
  //while (SERCOM1->SPI.SYNCBUSY.bit.CTRLB);  
  
  //Set up SPI interrupts
  SERCOM1->SPI.INTENSET.bit.SSL = 0x1; //Enable Slave Select low interrupt        
  SERCOM1->SPI.INTENSET.bit.RXC = 0x1; //Receive complete interrupt
  SERCOM1->SPI.INTENSET.bit.TXC = 0x1; //Receive complete interrupt
  SERCOM1->SPI.INTENSET.bit.ERROR = 0x1; //Receive complete interrupt
  SERCOM1->SPI.INTENSET.bit.DRE = 0x1; //Data Register Empty interrupt
  //init SPI CLK  
  //SERCOM1->SPI.BAUD.reg = SERCOM_FREQ_REF / (2*4000000u)-1;
  //Enable SPI
  SERCOM1->SPI.CTRLA.bit.ENABLE = 1;
  while(SERCOM1->SPI.SYNCBUSY.bit.ENABLE);
  SERCOM1->SPI.CTRLB.bit.RXEN = 0x1; //Enable Receiver, this is done here due to errate issue
  while(SERCOM1->SPI.SYNCBUSY.bit.CTRLB); //wait until receiver is enabled

}

void SERCOM1_Handler()
{
  //Serial.println("In SPI Interrupt"); 
  uint8_t data = 0;
  data =(uint8_t)SERCOM1->SPI.DATA.reg;
  //Serial.print("DATA currently in Register: "); Serial.print(data); Serial.print("\n");
  uint8_t interrupts = SERCOM1->SPI.INTFLAG.reg; //Read SPI interrupt register
  Serial.print("Interrupts:"); Serial.println(interrupts);
  if(interrupts & (1<<3))
  {
    //Serial.println("SPI SSL Interupt");
    SERCOM1->SPI.INTFLAG.bit.SSL = 1; //clear slave select interrupt
  }
  if(interrupts & (1<<2))
  {
    //Serial.println("SPI Data Received Complete Interrupt");
    data = SERCOM1->SPI.DATA.reg; //Read data register
    //Serial.print("DATA: "); Serial.println(data);
    SERCOM1->SPI.INTFLAG.bit.RXC = 1; //clear receive complete interrupt
    //Serial.print("DATA transferred Over: "); Serial.print(data); Serial.print("\n");
    Serial.println(data);
    logfile.println(data);
    logfile.flush();
  }
  if(interrupts & (1<<1))
  {
    //Serial.println("SPI Data Transmit Complete Interrupt");
    SERCOM1->SPI.INTFLAG.bit.TXC = 1; //clear receive complete interrupt
  }
  
  if(interrupts & (1<<0))
  {
    //Serial.println("SPI Data Register Empty Interrupt");
    SERCOM1->SPI.DATA.reg = 0xAA;
  }
 
  //Serial.print("CTRLA: "); Serial.println(SERCOM1->SPI.CTRLA.reg);  
}
  
