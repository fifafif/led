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
      
      /*int r = (color >> 16) & 0xff; // red
      int g = (int)((color >> 8) & 0xff); // green
      int b = (int)(color & 0xff);
      int v = r + g + b;
      v /= 3;*/

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
#if !defined(DMX_ON)
  Serial.println(message);
#endif
}

void log(String &message)
{
#if !defined(DMX_ON)
  Serial.println(message);
#endif
}
