#include <SPI.h>
#include "radio_config160_448_26.h"
//#include "radio_config160_450.h"
// pins used for the connection with the sensor
// the other you need are controlled by the SPI library):
unsigned long thistime;
unsigned long lasttime;
const int dataReadyPin = 6;
const int SDNPin = 9;
const int chipSelectPin = 10;
const int SDSelectPin = 9;
const int RXSelectPin = 4;
const int TXSelectPin = 5;
volatile boolean interuptwaiting;
volatile boolean channel;
unsigned char results[9];
unsigned char ADCresult[7];
unsigned char Status[5];
unsigned char interrupt[9];
byte changestate[3];
boolean dataReady;
unsigned char rxBuffer[10];
PROGMEM const uint8_t CONFIGURATION_DATA[] = RADIO_CONFIGURATION_DATA_ARRAY;

void setup() {
//  noInterrupts();
  int inputready=0;
  unsigned char reply;
  Serial.begin(9600);
  // start the SPI library:
  SPI.begin();
  // initalize the  data ready and chip select pins:
  pinMode(dataReadyPin, INPUT);
  pinMode(chipSelectPin, OUTPUT);
  pinMode(SDSelectPin, OUTPUT);
  pinMode(SDNPin, OUTPUT);
  pinMode(RXSelectPin, OUTPUT);
  pinMode(TXSelectPin, OUTPUT);
  digitalWrite(RXSelectPin, LOW);
  digitalWrite(TXSelectPin, LOW);
  pinMode(2,INPUT);
  digitalWrite(SDSelectPin, HIGH);
  delay(100);
  digitalWrite(SDSelectPin, LOW); 
  //Serial.println("setup start");
  //delay(100);

   interuptwaiting=0;
   attachInterrupt(digitalPinToInterrupt(2),si4464interupt,FALLING);
   delay(500);
   interuptwaiting=0;
   /*RF_POWER_UP 30Mhz Crystal
   //writeRegister(7,(const byte[]){0x02, 0x01, 0x00, 0x01, 0xC9, 0xC3, 0x80});
    //RF_POWER_UP 26Mhz Crystal
   //writeRegister(7,(const byte[]){0x02, 0x01, 0x00, 0x01, 0x8C, 0xBA, 0x80});  
    while (interuptwaiting==0){
      digitalWrite(chipSelectPin, LOW);
      delayMicroseconds(2);
      SPI.transfer(0x44);
      delayMicroseconds(2);
      reply = SPI.transfer(0x00);
      delayMicroseconds(2);
      if (reply != 0xFF){
        digitalWrite(chipSelectPin, HIGH);
        delayMicroseconds(20);
      }else{
        interuptwaiting=1;   
      }

    } //0x04,0x04,0x00,0x02,0x00,0x00,0x14,0x14,0x00
    */
  digitalWrite(chipSelectPin, HIGH);
   configure(CONFIGURATION_DATA);
   waitforreply();
   writeRegister(2,(const byte[]){0x34, 0x01});
   waitforreply();
   writeRegister(2,(const byte[]){0x15, 0x03});
   waitforreply();
   digitalWrite(TXSelectPin, HIGH);
   delay (200);
   writeRegister(5,(const byte[]){0x31, 0x00, 0x00, 0x00, 0x00});
   waitforreply();

   Serial.print("STARTUP DONE");
   lasttime= millis();
}

void loop() {
  unsigned char reply;
   float temprature;
   float battery;
   thistime= millis();
   if ((thistime-lasttime)>5000){
  // writeRegister(5,(const byte[]){0x31, 0x00, 0x00, 0x00, 0x00});
  // waitforreply();
    lasttime=thistime;
      writeRegister(3,(const byte[]){0x14,0x18,0x00}); //reply 0x04,0x04,0x00,0x02,0x00,0x00,0x14,0x14,0x00
     while (reply != 0xFF)
     {
       digitalWrite(chipSelectPin, LOW);
       delayMicroseconds(2);
       SPI.transfer(0x44);
       delayMicroseconds(2);
       reply = SPI.transfer(0x00);
       delayMicroseconds(2);
       if (reply != 0xFF){
         digitalWrite(chipSelectPin, HIGH);
         delayMicroseconds(20);
       }
     }
     delayMicroseconds(2);
     for (int j = 0; j < 7; j++) // Loop through the bytes of the pData
     {
         ADCresult[j]=SPI.transfer(0x00);
     }
     digitalWrite(chipSelectPin, HIGH);

      writeRegister(2,(const byte[]){0x23,0x00}); //reply 0x04,0x04,0x00,0x02,0x00,0x00,0x14,0x14,0x00
     while (reply != 0xFF)
     {
       digitalWrite(chipSelectPin, LOW);
       delayMicroseconds(2);
       SPI.transfer(0x44);
       delayMicroseconds(2);
       reply = SPI.transfer(0x00);
       delayMicroseconds(2);
       if (reply != 0xFF){
         digitalWrite(chipSelectPin, HIGH);
         delayMicroseconds(20);
       }
     }
     delayMicroseconds(2);
     for (int j = 0; j < 5; j++) // Loop through the bytes of the pData
     {
         Status[j]=SPI.transfer(0x00);
     }
     Serial.println();
     digitalWrite(chipSelectPin, HIGH);
     for (int j = 0; j < 5; j++) // Loop through the bytes of the pData
     {
         Serial.print(Status[j],HEX);
         Serial.print(" ");
     }     
     Serial.println();
     for (int j = 0; j < 7; j++) // Loop through the bytes of the pData
     {
         Serial.print(ADCresult[j],HEX);
         Serial.print(" ");
     }
     Serial.println();
     uint16_t temp_adc = (ADCresult[4] << 8) | ADCresult[5];
     temprature=((800 + ADCresult[6]) / 4096.0) * temp_adc - ((ADCresult[7] / 2) + 256);
     Serial.println(temprature);
     uint16_t battery_adc = (ADCresult[2] << 8) | ADCresult[3];
     battery = 3.0 * battery_adc / 1280;
     Serial.println(battery);
     //writeRegister(5,(const byte[]){0x31, 0x00, 0x00, 0x00, 0x00});
   
   }
   writeRegister(2,(const byte[]){0x34, 0x01});
   waitforreply();
   writeRegister(2,(const byte[]){0x15, 0x03});
   waitforreply();
   digitalWrite(RXSelectPin, LOW);
   digitalWrite(TXSelectPin, HIGH);
   delay (200);
   writeRegister(5,(const byte[]){0x31, 0x00, 0x00, 0x00, 0x00});
   waitforreply();
   delay (200);
   digitalWrite(TXSelectPin, LOW);
   digitalWrite(RXSelectPin, HIGH);
  
   
}

bool configure(const uint8_t* commands)
{
    // Command strings are constructed in radio_config_Si4460.h 
    // Each command starts with a count of the bytes in that command:
    // <bytecount> <command> <bytecount-2 bytes of args/data>
    uint8_t next_cmd_len;
    uint8_t read_buf[8];
    
    while (memcpy_P(&next_cmd_len, commands, 1), next_cmd_len > 0)
    {
      uint8_t buf[20]; // As least big as the biggest permitted command/property list of 15
      memcpy_P(buf, commands+1, next_cmd_len);
      command(buf[0], buf+1, next_cmd_len - 1,read_buf, sizeof(read_buf));
      commands += (next_cmd_len + 1);
    }
    return true;
}
void writeRegister(int byteCountTx, const byte* pData) {

 //int byteCountTx= sizeof(pData);
  // Serial.print(byteCountTx, HEX);
  // take the chip select low to select the device:
  digitalWrite(chipSelectPin, LOW);
   delayMicroseconds(2);
    for (int j = 0; j < byteCountTx; j++) // Loop through the bytes of the pData
    {
      byte wordb = pData[j];
      SPI.transfer(wordb);  
    }
   delayMicroseconds(2);
  // take the chip select high to de-select:
  digitalWrite(chipSelectPin, HIGH);
}

void waitforreply(){
  unsigned char reply;
    reply=0;
    while (reply != 0xFF)
    {
      digitalWrite(chipSelectPin, LOW);
      delayMicroseconds(2);
      SPI.transfer(0x44);
      delayMicroseconds(20);
      reply = SPI.transfer(0x00);
      delayMicroseconds(20);
      digitalWrite(chipSelectPin, HIGH);
      delayMicroseconds(2);
    }
    delayMicroseconds(2);
}

void waitforreplyread(){
  unsigned char reply;
    reply=0;
    while (reply != 0xFF)
    {
      digitalWrite(chipSelectPin, LOW);
      delayMicroseconds(2);
      SPI.transfer(0x44);
      delayMicroseconds(2);
      reply = SPI.transfer(0x00);
      delayMicroseconds(2);
      if (reply != 0xFF){
        digitalWrite(chipSelectPin, HIGH);
        delayMicroseconds(20);
      }
    }
    delayMicroseconds(2);
}
 
void si4464interupt() {
  unsigned char reply;
  reply=0;
  //void waitforreply();
  writeRegister(4,(const byte[]){0x20,0x00,0x00,0x00 }); //reply 0x04,0x04,0x00,0x02,0x00,0x00,0x14,0x14,0x00
  while (reply != 0xFF)
    {
      digitalWrite(chipSelectPin, LOW);
      SPI.transfer(0x44);
      reply = SPI.transfer(0x00);
      if (reply != 0xFF){
        digitalWrite(chipSelectPin, HIGH);
      }
    }
  for (int j = 0; j < 9; j++) // Loop through the bytes of the pData
     {
         interrupt[j]=SPI.transfer(0x00);
     }
   digitalWrite(chipSelectPin, HIGH);
   interuptwaiting =1;
}
bool command(uint8_t cmd, const uint8_t* write_buf, uint8_t write_len, uint8_t* read_buf, uint8_t read_len)  
{  
    bool done = false;  
    unsigned char reply;
    //ATOMIC_BLOCK_START;  
    // First send the command  
    digitalWrite(chipSelectPin, LOW);  
    SPI.transfer(cmd);  
  
    // Now write any write data  
    if (write_buf && write_len)  
    {  
    while (write_len--)  
        SPI.transfer(*write_buf++);  
    }  
    // Sigh, the RFM26 at least has problems if we deselect too quickly :-(  
    // Innocuous timewaster:  
    digitalWrite(chipSelectPin, LOW);  
    // And finalise the command  
    digitalWrite(chipSelectPin, HIGH);  
  
    uint16_t count; // Number of times we have tried to get CTS  
    for (count = 0; !done && count < 2500; count++)  
    {  
    // Wait for the CTS  
    digitalWrite(chipSelectPin, LOW);  
  
    SPI.transfer(0x44);
    reply = SPI.transfer(0x00);
    if (reply == 0xFF){   
        // Now read any expected reply data  
        if (read_buf && read_len)  
        {  
        while (read_len--)  
        {  
            *read_buf++ = SPI.transfer(0x00);  
        }  
        }  
        done = true;  
    }  
    // Sigh, the RFM26 at least has problems if we deselect too quickly :-(  
    // Innocuous timewaster:  
    digitalWrite(chipSelectPin, LOW);  
    // Finalise the read  
    digitalWrite(chipSelectPin, HIGH);  
    }  
    //ATOMIC_BLOCK_END;  
    return done; // False if too many attempts at CTS  
}
