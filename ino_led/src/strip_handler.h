#ifndef STRIP_HANDLER_H
#define STRIP_HANDLER_H

#if !defined(LED_SIM_ONLY)

#if defined(LED_NEOPIXEL)
#include <Adafruit_NeoPixel.h>
#else
#include <LiteLED.h>
#endif

#endif

#include <Arduino.h>
#include "colors.h"

class StripHandler {
  public:

    int pixelCount;
    float brightness;
    byte redValue = 255;
    byte greenValue = 255;
    byte blueValue = 0;
    byte colorWheelPosition;
    byte *stripValues;

    #if defined(LED_SIM_ONLY)
    uint32_t *strip;
    #else
    #if defined(LED_NEOPIXEL)
    Adafruit_NeoPixel *strip;
    #else
    LiteLED *strip;
    #endif
    #endif

    void maxValue(int i, float value)
    {
      byte valueByte = byte(value * 255);
      if (valueByte < stripValues[i]) return;

      stripValues[i] = valueByte;
      setPixelColor(i, value);
    }

    void addValue(int i, float value)
    {
      int sum = (int)(value * 255.0f) + stripValues[i];
      if (sum > 255)
      {
        sum = 255;
      }
      else if (sum < 0)
      {
        sum = 0;
      }

      stripValues[i] = (byte)sum;
      setPixelColor(i, clamp01(sum / 255.0f));
    }

    void setValue(int i, float value)
    {
      byte valueByte = byte(value * 255);
      
      stripValues[i] = valueByte;
      setPixelColor(i, value);
    }

    void setValueByte(int i, byte value)
    {
      stripValues[i] = value;
      setPixelColor(i, value / 255.0f);
    }

    void fadeValue(int i, byte fade)
    {
      byte value = stripValues[i];
      if (value < fade)
      {
        value = 0;
      }
      else
      {
        value -= fade;
      }

      setValueByte(i, value);
    }

    void fadeValueOnly(int i, byte fade)
    {
      byte value = stripValues[i];
      if (value < fade)
      {
        value = 0;
      }
      else
      {
        value -= fade;
      }

      stripValues[i] = value;
    }

    void setPixelColor(int i, float c)
    {
      c = clamp01(c);
      c *= brightness;
      setPixelColor(i, getColor(c * redValue, c * greenValue, c * blueValue));
    }

    void setPixelColor(int i, uint32_t color)
    {
    #if defined(LED_SIM_ONLY)
      strip[i] = color;
    #else

      color = color & 0x00FFFFFF;

    #if defined(LED_NEOPIXEL)
      strip->setPixelColor(i, color);
    #else
      strip->setPixel(i, color, 0);
    #endif

    #endif
    }

    uint32_t getPixelColor(int i)
    {
    #if defined(LED_SIM_ONLY)
      return strip[i];
    #else

    #if defined(LED_NEOPIXEL)
      return strip->getPixelColor(i);
    #else
      return strip->getPixelC(i);
    #endif

    #endif
    }

    void setRGBFromWheel(byte wheelPos)
    {
      colorWheelPosition = wheelPos;
      rgbFromWheel(wheelPos, redValue, greenValue, blueValue);

      #if defined(LED_SIM_ONLY)
      logColor(redValue, greenValue, blueValue);      
      #endif
    }

    void generateRandomStripValues()
    {
      for (int i = 0; i < pixelCount; i++)
      {
        stripValues[i] = random(255);
      }
    }

    void clearRandomStripValues()
    {
      for (int i = 0; i < pixelCount; i++)
      {
        stripValues[i] = 0;
      }
    }

    void clearColor()
    {
      setColorToAll((uint32_t)0);
    }

    void setValueToAll(byte value)
    {
      for (int i = 0; i < pixelCount; i++)
      {
        stripValues[i] = value;
      }
    }

    void setColorToAll(uint32_t color)
    {
      for (int i = 0; i < pixelCount; i++)
      {
        setPixelColor(i, color);
      }
    }

    void setColorToAll(float c)
    {
      c *= brightness;
      uint32_t color = getColor(c * redValue, c * greenValue, c * blueValue);
      for (int i = 0; i < pixelCount; i++)
      {
        setPixelColor(i, color);
      }
    }

    void setValueAndColorToAll(float c)
    {
      byte value = byte(c * 255);
      c *= brightness;
      uint32_t color = getColor(c * redValue, c * greenValue, c * blueValue);
      for (int i = 0; i < pixelCount; i++)
      {
        stripValues[i] = value;
        setPixelColor(i, color);
      }
    }

    void testSequence()
    {
      for (int i = 0; i < pixelCount; i++)
      {
        setPixelColor(i, getColor(256.0f * i / pixelCount, 0, 0));
      }
    }
};

#endif