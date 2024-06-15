#ifndef ANIMATION_H
#define ANIMATION_H

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
    bool isStepStartChangingColor = false;
    float beatDuration = 1;
    unsigned long beatTimeMs;

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

    virtual void onStepStart()
    {
      logLedSim("onStepStart: ", playback->sequenceStep);
    }

    void beat()
    {
      beatTimeMs = getMs();
      onBeat();
    }

    virtual void onBeat()
    {
      logLedSim("beat");
    }
    
    int reverseIndex(int i)
    {
      return strip->pixelCount - i - 1;
    }

    void updateSegment(float seqValues[], int seqLength)
    {
      playback->updateStepTime(6.0f);

      playback->ledIndex = quadraticEaseInOut(playback->normalizedStepTime) * playback->pixelCount;
      
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

    float getElapsedBeatFactor()
    {
      return (getMs() - beatTimeMs) * 0.001 / beatDuration;
    }

    void centerGlowBeat(int beatLength, float beatValue)
    {
      for (int i = 0; i < beatLength; i++)
      {
        float c = 1.0f - easeIn((float)i / beatLength);
        c *= beatValue;
        if (c * 255 > strip->stripValues[i])
        {
          strip->setPixelColor(i, c);
        }
      }
    }

    void centerGlowBeat(int beatLength, float beatValue, float duration, bool isReverse = false)
    {
      if (duration > 0)
      {
        beatLength *= 1 - clamp01(getElapsedBeatFactor() / duration);
      }

      int start = isReverse ? playback->pixelCount - beatLength : 0;
      int end = isReverse ? playback->pixelCount : beatLength;

      for (int i = start; i < end; i++)
      {
        float c = 1.0f - easeIn((float)i / beatLength);
        if (isReverse)
        {
          c = 1 - c;
        }

        c *= beatValue;

        if (c * 255 > strip->stripValues[i])
        {
          strip->setPixelColor(i, c);
        }
      }
    }
};

#endif