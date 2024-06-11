#ifndef ANIMATION_IMPLEMENTATIONS_H
#define ANIMATION_IMPLEMENTATIONS_H

#include <Arduino.h>
#include "debug.h"
#include "animations.h"
#include "sequences.h"
#include "colors.h"
#include "playback.h"

class Animation
{
  public:
    Playback *playback;
    StripHandler *strip;

    Animation(Playback *playback, StripHandler *strip)
    {
      this->playback = playback;
      this->strip = strip;
    }

    virtual void update(){
      logAlways("animation update");
    }

    void onStart()
    {
      log("onStart");
    }

    void onSequenceEnd()
    {
      log("onSequenceEnd");
    }
};

class FireboltAnimation : public Animation
{
  public:
    int length;

    FireboltAnimation(Playback *playback, StripHandler *strip, int length) : Animation(playback, strip)
    {
      this->length = length;
    }

    void update()
    {
      if (playback->isTickEnd)
      {
        strip->generateRandomStripValues();
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
          byte value = strip->stripValues[i];
          value -= 10;
          strip->stripValues[i] = value;
          c *= value / 255.0;
        }

        strip->setPixelColor(i, clamp01(c));
      }
    }
};

class CylonAnimation : public Animation
{
  public:
    int length;

    CylonAnimation(Playback *playback, StripHandler *strip, int length) : Animation(playback, strip)
    {
      this->length = length;
    }

    void update()
    {
      cylon(length);
    }

    void cylon(int length)
    {
      /*Serial.print("cylon ");
      Serial.print(playback->pixelCount);
      Serial.print(" - ");
      Serial.print(playback->normalizedStepTime);
      Serial.println(" end");*/
      
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