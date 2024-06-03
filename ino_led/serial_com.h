#include <Arduino.h>

HardwareSerial SerialPort(2); // use UART2

bool isSerialSlave;
int ledPin = 2;
const int bufferSize = 3;
byte buffer[bufferSize] = {};

byte getSerialMessageType()
{
  return buffer[0];
}

byte getSerialMessageSequence()
{
  return buffer[1];
}

byte getSerialMessageColor()
{
  return buffer[2];
}

void setupSerial(bool isSlave)
{
  isSerialSlave = isSlave;
  Serial.print("Begin! is slave: ");
  Serial.println(isSerialSlave);
  SerialPort.begin(115200, SERIAL_8N1, 16, 17);
}

void writeSerial(byte value)
{
#if !defined(LED_SIM_ONLY)
  Serial.print("[Serial] write value=");
  Serial.println(value);
#endif

  SerialPort.print(value);
}

void writeSerialBuffer()
{
#if !defined(LED_SIM_ONLY)
  Serial.print("[Serial] write buffer. size=");
  Serial.println(bufferSize);
#endif

  SerialPort.write(buffer, bufferSize);
}

void writeSerialMessage(byte type, byte sequence, byte color)
{
  buffer[0] = type;
  buffer[1] = sequence;
  buffer[2] = color;

  writeSerialBuffer();
}

void writeSerialMessageColor(byte color)
{
  writeSerialMessage(2, 0, color);
}

void writeSerialMessageOverdrive(byte sequence, byte color)
{
  writeSerialMessage(3, sequence, color);
}

bool readSerialBuffer()
{
  if (SerialPort.available())
  {
    int size = SerialPort.readBytes(buffer, bufferSize);

#if !defined(LED_SIM_ONLY)
    Serial.print("[Serial] read buffer available! size=");
    Serial.println(size);
#endif

    return true;
  }

  return false;
}

bool readSerialMessage()
{
  if (!readSerialBuffer())
  {
    return false;
  }

  return true;
}

void readSerialChar()
{
  if (SerialPort.available())
  {
    Serial.print("[Serial] read available!");

    char number = SerialPort.read();
    Serial.println(number);
    if (number == '0') {
      digitalWrite(ledPin, LOW);
    }
    if (number == '1') {
      digitalWrite(ledPin, HIGH);
    }
  }
}