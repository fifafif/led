#ifndef STRIP_HANDLER_H
#define STRIP_HANDLER_H

#include <Arduino.h>
#include "colors.h"

class StripHandler {
  public:

    int numPixels;
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

    void setPixelColor(int i, float c)
    {
      c *= brightness;
      setPixelColor(i, getColor(c * redValue, c * greenValue, c * blueValue));
    }

    void setPixelColor(int i, uint32_t color)
    {
    #if defined(LED_SIM_ONLY)
      strip[i] = color;
    #else

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
    }

    void generateRandomStripValues()
    {
      for (int i = 0; i < numPixels; i++)
      {
        stripValues[i] = random(255);
      }
    }

    void testSequence()
    {
      for (int i = 0; i < numPixels; i++)
      {
        setPixelColor(i, getColor(256.0f * i / numPixels, 0, 0));
      }
    }
};

#endif