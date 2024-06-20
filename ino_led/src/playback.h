#ifndef PLAYBACK_H
#define PLAYBACK_H

#include <Arduino.h>
#include "debug.h"

unsigned long getMs()
{
  #if defined(LED_NEOPIXEL)
  return (unsigned long)(millis() * 1.25);
  #else
  return millis();
  #endif
}

class Playback
{
  public:
    int ledIndex;
    int stepTicks;
    float normalizedStepTime;
    float deltaTime;
    int sequenceStep;
    unsigned long stepStartMs;
    float lastUpdateTime;
    bool isSequenceEnd;
    bool isStepEnd;
    bool isSlave;
    int pixelCount;
    int animationPlayCount;
    byte speed = 127;

    Playback(int pixelCount)
    {
      this->pixelCount = pixelCount;
    }

    bool updateStepTime(float stepDuration)
    {
      return updateStepTime(stepDuration, true);
    }

    bool updateStepTime(float stepDuration, int stepCount)
    {
      if (updateStepTime(stepDuration, false))
      {
        if (sequenceStep >= stepCount)
        {
          sequenceEnd();
        }
        // else if (isChangingColor)
        // {
        //   if (!isSlave)
        //   {
        //     // changeColor();
        //     // writeSerialColor();
        //   }
        // }
        return true;
      }

      return false;
    }

    bool updateStepTime(float stepDuration, bool isLastStep)
    {
      float speedFactor = (float)speed / 127.0f;
      unsigned long ms = getMs();
      deltaTime = 0.001f * (ms - lastUpdateTime) * speedFactor;
      lastUpdateTime = ms;
      unsigned long elapsedMs = ms - stepStartMs;
      normalizedStepTime = elapsedMs / (stepDuration * 1000);
      normalizedStepTime *= speedFactor;
      ledIndex = (int)(normalizedStepTime * pixelCount) % pixelCount;
      isStepEnd = false;
      isSequenceEnd = false;
      stepTicks += 1;

      if (normalizedStepTime >= 1)
      {
        stepTimeEnd();
        if (isLastStep)
        {
          sequenceEnd();
        }
        return true;
      }

      return false;
    }

    void stepTimeEnd()
    {
      String msg = "step end. ms=";
      msg = msg + getMs();
      logLedSim(msg);
      startStepTime();
      sequenceStep += 1;
      isStepEnd = true;
    }

    void startStepTime()
    {
      ledIndex = 0;
      stepTicks = 0;
      normalizedStepTime = 0;
      stepStartMs = getMs();
      lastUpdateTime = stepStartMs;
    }
        
    void sequenceEnd()
    {
      /*if (IS_SLAVE)
      {
        return;
      }*/

      logLedSim("sequence end");
      
      if (isSlave)
      {
        //slaveColorMode = random(slaveColorModeCount);
      }
      
      ledIndex = 0;
      isSequenceEnd = true;
      sequenceStep = 0;
      animationPlayCount += 1;

      if (!isSlave)
      {
        // writeSerialMessage(1, randomMode, colorWheelPosition);
      }
    }

    void moveTime(float duration)
    {
      stepStartMs += duration * 1000;
    }
};
#endif