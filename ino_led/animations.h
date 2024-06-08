#ifndef ANIMATIONS_H
#define ANIMATIONS_H

#include <Arduino.h>
#include "tick_payload.h"
#include "sequences.h"
#include "colors.h"
#include "playback.h"

class Animations
{
  public:
    Playback *playback;
    StripHandler *strip;
    byte *stripValues;

    Animations(Playback *playback, StripHandler *stripHandler, byte *stripValues)
    {
      this->playback = playback;
      this->strip = stripHandler;
      this->stripValues = stripValues; 
    }

    void update()
    {
      //cylon(100);
      firebolt(100);
    }

    
    void generateRandomStripValues()
    {
      for (int i = 0; i < playback->pixelCount; i++)
      {
        stripValues[i] = random(255);
      }
    }

    void firebolt(int length)
    {
      if (playback->isTickEnd)
      {
        generateRandomStripValues();
      }

      if (playback->updateStepTime(2.0f, true)) return;

      playback->ledIndex = easeOut(playback->normalizedStepTime) * playback->pixelCount;

      for (int i = 0; i < playback->pixelCount; i++)
      {
        float c = inverseLerp(playback->ledIndex - length, playback->ledIndex, i);
        c = easeOut(c);
        if (i > playback->ledIndex)
        {
          c = 0;
        }
        else
        {
          byte value = stripValues[i];
          value -= 10;
          stripValues[i] = value;
          c *= value / 255.0;
        }

        strip->setPixelColor(i, clamp01(c));
      }
    }

    void cylon(int length)
    {
      Serial.print("cylon ");
      Serial.print(playback->pixelCount);
      Serial.print(" - ");
      Serial.print(playback->normalizedStepTime);
      Serial.println(" end");
      
      if (playback->updateStepTime(2.0f, true)) return;

      bool isReverse = playback->normalizedStepTime > 0.5;
      if (isReverse)
      {
        playback->normalizedStepTime -= 0.5;
      }

      playback->normalizedStepTime *= 2;

      int ledIndex = easeOut(playback->normalizedStepTime) * playback->pixelCount;
      length = lerp(length / 2, length, playback->normalizedStepTime);

      for (int i = 0; i < playback->pixelCount; i++)
      {
        float c = inverseLerp(ledIndex - length, ledIndex, i);
        c = easeIn(c);
        if (i > ledIndex)
        {
          c = 0;
        }

        if (isReverse) {
          strip->setPixelColor(i, c);
        } else {
          strip->setPixelColor(playback->pixelCount - i - 1, c);
        }
      }
    }
};

#endif
