#ifndef ANIMATION_IMPLEMENTATIONS_H
#define ANIMATION_IMPLEMENTATIONS_H

#include <Arduino.h>
#include "debug.h"
#include "animations.h"
#include "sequences.h"
#include "colors.h"
#include "playback.h"

#define PI 3.14159
#define PI2 2 * PI

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
      logLedSim("animation update");
    }

    virtual void onStart()
    {
      logLedSim("onStart");
      strip->clearRandomStripValues();
    }

    virtual void onSequenceEnd()
    {
      logLedSim("onSequenceEnd");
    }

    virtual void onSequenceStart()
    {
      logLedSim("onSequenceStart");
    }
    
    int reverseIndex(int i)
    {
      return strip->pixelCount - i - 1;
    }

    void updateSegment(float seqValues[], int seqLength)
    {
      playback->updateStepTime(6.0f, true);

      playback->ledIndex = easeInSine(playback->normalizedStepTime) * playback->pixelCount;
      
      for (int i = 0; i < playback->pixelCount; i++)
      {
        int ii = i - playback->ledIndex;
        if (ii < 0)
        {
          ii += playback->pixelCount;
        }
      
        float c = seqValues[ii % seqLength];

        strip->setPixelColor(i, c);
      }
    }
};

class SegmentAnimation : public Animation
{
  public:
    int length;
    byte mode;

    SegmentAnimation(Playback *playback, StripHandler *strip, byte mode, int length) : Animation(playback, strip)
    {
      this->mode = mode;
      this->length = length;
    }

    void update()
    {
      switch (mode)
      {
        case 1:
          wavesHardSeq();
          break;

        default: // 0
          wavesSeq();
      }
    }

    void wavesHardSeq()
    {
      float s[length];
      for (int i = 0; i < length; ++i)
      {
        s[i] = i > length / 2 ? 1 : 0;
      }

      updateSegment(s, length);
    }

    void wavesSeq()
    {
      float s[length];
      for (int i = 0; i < length; ++i)
      {
        float v = sin(PI * float(i) / length);
        v = v * 1.2 - 0.2;
        s[i] = v > 0 ? v : 0;
      }

      updateSegment(s, length);
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

    void onSequenceStart()
    {
      log("Firebolt onSequenceStart");
      strip->generateRandomStripValues();
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