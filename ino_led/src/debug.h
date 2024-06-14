#ifndef DEBUG_H
#define DEBUG_H

#include <Arduino.h>

char* brightnessToAscii(byte brightness)
{
  const int stepCount = 5;
  // byte value = (byte)round(1.0 * brightness * (stepCount - 1) / 255);
  byte value = brightness * (stepCount - 1) / 255;
  switch (value)
  {
    case 0: return "_";
    case 1: return "\u2591";
    case 2: return "\u2592";
    case 3: return "\u2593";
    case 4: return "\u2588";
    default: return "?";
  }
}

void testAscii()
{
  int steps = 10;
  for (int i = 0; i < steps; i++)
  {
    byte brightness = (byte)(i * 255 / (steps - 1)); 
    Serial.print(brightnessToAscii(brightness));
  }
  
  Serial.println("end test");
}

void logStripBytes(uint32_t *strip, int length)
{
  for (int i = 0; i < length; i++)
  {
    uint32_t color = strip[i];

#if defined(LED_SIM_PRINT_BYTES_BRIGHTNESS)   
    byte b = (byte)((color >> 16) & 0xff); // red
    b = max(b, (byte)((color >> 8) & 0xff)); // green
    b = max(b, (byte)(color & 0xff));

    Serial.write(b);
#else
    byte r = (byte)((color >> 16) & 0xff);
    byte g = (byte)((color >> 8) & 0xff);
    byte b = (byte)((color >> 0) & 0xff);

    Serial.write(r);
    Serial.write(g);
    Serial.write(b);
#endif    
  }

  Serial.println();
}

void logColor(byte r, byte b, byte g)
{
    Serial.print('#');
    Serial.write(r);
    Serial.write(g);
    Serial.write(b);
    Serial.println();
}

void logStrip(uint32_t *strip, int length)
{
#if defined(LED_SIM_ONLY) 

  int width = 100;
  float step = 1.0 * length / width;
  int ledIndex = 0;
  float ledIndexFloat = 0;

  Serial.print("[");

  for (int i = 0; i < width; i++)
  {
    ledIndexFloat += step;
    int prevIndex = ledIndex;
    ledIndex = (int)round(ledIndexFloat);
    int brightness = 0;

    for (int j = prevIndex; j < ledIndex; j++)
    {
      uint32_t color = strip[j];
      int v = (color >> 16) & 0xff; // red
      v = max(v, (int)((color >> 8) & 0xff)); // green
      v = max(v, (int)(color & 0xff));
      
      brightness += v;
    }

    brightness /= ledIndex - prevIndex;

    Serial.print(brightnessToAscii((byte)brightness));
    //Serial.print((byte)brightness);
    //Serial.print("-");
  }

  Serial.println("]");

#endif
}

void log(char *message)
{
#if !defined(DMX_ON) && !defined(LED_SIM_ONLY)
  Serial.println(message);
#endif
}

void log(String &message)
{
#if !defined(DMX_ON) && !defined(LED_SIM_ONLY)
  Serial.println(message);
#endif
}

void logLedSim(String &message)
{
#if defined(LED_SIM_ONLY) && defined(LED_SIM_DEBUG)
  Serial.println(message);
#endif
}

void logLedSim(char *message)
{
#if defined(LED_SIM_ONLY) && defined(LED_SIM_DEBUG)
  Serial.println(message);
#endif
}

void logNumbers(float num1, float num2)
{
  Serial.print(num1);
  Serial.print("-");
  Serial.println(num2);
}

void logNumbers(char* msg, float num1, float num2)
{
  Serial.print(msg);
  Serial.print(num1);
  Serial.print("-");
  Serial.println(num2);
}

void logNumbers(float num1, float num2, float num3)
{
  Serial.print(num1);
  Serial.print("-");
  Serial.print(num2);
  Serial.print("-");
  Serial.println(num3);  
}

void printArray(int *array, int size)
{
  for (int i = 0; i < size - 1; i++)
  {
    Serial.print(array[i]);
    Serial.print(",");
  }

  if (size > 0)
  {
    Serial.println(array[size - 1]);
  }
}

#endif