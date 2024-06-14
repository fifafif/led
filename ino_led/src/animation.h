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
      
    }
    
    int reverseIndex(int i)
    {
      return strip->pixelCount - i - 1;
    }

    void updateSegment(float seqValues[], int seqLength)
    {
      playback->updateStepTime(6.0f, true);

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
};

#endif