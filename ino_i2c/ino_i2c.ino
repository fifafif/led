#include <Wire.h>
#include <Arduino.h>
 
const bool IS_SLAVE = false; 

#define I2C_SDA 21
#define I2C_SCL 22

void setup() {
  //Wire.begin();

  if (IS_SLAVE)
  {
    Wire.begin(0x33);  
  }
  else
  {
    Wire.begin(I2C_SDA, I2C_SCL, 0x11);
  }
  Serial.begin(115200);
  Serial.println("\nI2C Scanner");
}
 
void loop() 
{
  if (IS_SLAVE)
  {
    Serial.println("Slaving on...");
    delay(1000);
  }
  else
  {
    scan();
    delay(1000);
  }
}

void scan()
{
  byte error, address;
  int nDevices;
  Serial.println("Scanning...");
  nDevices = 0;
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
      nDevices++;
    }
    else if (error==4) {
      Serial.print("Unknow error at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
    }  
    else if (error != 2)
    {
      Serial.print("error=");
      Serial.print(error);
      Serial.print(",");
    }  
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
  }
  else {
    Serial.println("done\n");
  }
}