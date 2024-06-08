#ifndef STRIP_HANDLER_H
#define STRIP_HANDLER_H

#include "colors.h"

class StripHandler {
  public:

    int numPixels;
    float brightness;
    byte redValue = 255;
    byte greenValue = 255;
    byte blueValue = 0;
    byte colorWheelPosition;

    #if defined(LED_SIM_ONLY)
    uint32_t *strip;//[NUMPIXELS];
    #else
    #if defined(LED_NEOPIXEL)
    Adafruit_NeoPixel *strip;//(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
    #else
    LiteLED *strip;//(LED_STRIP_WS2812, 0);
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

    void testSequence()
    {
      for (int i = 0; i < numPixels; i++)
      {
        setPixelColor(i, getColor(256.0f * i / numPixels, 0, 0));
      }
    }
};

#endif