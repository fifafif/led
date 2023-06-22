#include <HardwareSerial.h>

#define LED_ONBOARD 2

const bool IS_SLAVE = true; 

HardwareSerial SerialPort(2); // use UART2

char number  = ' ';
int LED = 15;

void setup()  
{
  Serial.begin(115200);
  Serial.print("Begin! is slave: ");
  Serial.println(IS_SLAVE);
  SerialPort.begin(15200, SERIAL_8N1, 16, 17);
  pinMode(LED_ONBOARD, OUTPUT); 
} 

void loop()  
{ 
  if (IS_SLAVE)
  {
    Serial.println("Slaving on...");
    readSerial();
    delay(1000);
  }
  else
  {
    writeSerial();
    delay(1000);
  }
}

void writeSerial()
{
  writeValue(1);
  delay(1000);
  writeValue(0);
  delay(1000);
}

void writeValue(byte value)
{
  Serial.print("write value=");
  Serial.println(value);
  SerialPort.print(value);
}

void readSerial()
{
  if (SerialPort.available())
  {
    Serial.print("read available!");

    char number = SerialPort.read();
    Serial.println(number);
    if (number == '0') {
      digitalWrite(LED_ONBOARD, LOW);
    }
    if (number == '1') {
      digitalWrite(LED_ONBOARD, HIGH);
    }
  }
}