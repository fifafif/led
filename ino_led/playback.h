#ifndef PLAYBACK_H
#define PLAYBACK_H

#include <Arduino.h>
#include "debug.h"

class Playback
{
  public:
    int ledIndex;
    int stepTicks;
    float normalizedStepTime;
    int sequenceStep;
    unsigned long stepStartMs;
    bool isTickEnd;
    bool isSlave;
    int pixelCount;
    int tickCount;

    Playback(int pixelCount)
    {
      this->pixelCount = pixelCount;
    }

    bool updateStepTime(float stepDuration, int stepCount, bool isChangingColor)
    {
      if (updateStepTime(stepDuration))
      {
        if (stepCount <= sequenceStep)
        {
          sequenceEnd();
        }
        else if (isChangingColor)
        {
          if (!isSlave)
          {
            // changeColor();
            // writeSerialColor();
          }
        }
        return true;
      }

      return false;
    }

    bool updateStepTime(float stepDuration)
    {
      return updateStepTime(stepDuration, false);
    }

    bool updateStepTime(float stepDuration, bool isLastStep)
    {
      unsigned long ms = getMs();
      unsigned long elapsedMs = ms - stepStartMs;
      normalizedStepTime = elapsedMs / (stepDuration * 1000);
      ledIndex = (int)(normalizedStepTime * pixelCount) % pixelCount;
      isTickEnd = false;
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
      log(msg);
      startStepTime();
      sequenceStep += 1;
      tickCount += 1;
    }

    void startStepTime()
    {
      ledIndex = 0;
      stepTicks = 0;
      normalizedStepTime = 0;
      stepStartMs = getMs();
    }
        
    void sequenceEnd()
    {
      /*if (IS_SLAVE)
      {
        return;
      }*/

      log("sequence end");
      
      if (isSlave)
      {
        //slaveColorMode = random(slaveColorModeCount);
      }
      
      //changeColor();
      ledIndex = 0;
      isTickEnd = true;
      sequenceStep = 0;

      /*
      if (isOverdrive)
      {
        log("overdrive end");
        isOverdrive = false;
        choseRandomSequence();
        return;
      }*/

      /*
      if (mode == 255
          && tickCount % switchAutoModeEveryTickCount == 0)
      {
        choseRandomSequence();
      }*/

      if (!isSlave)
      {
        // writeSerialMessage(1, randomMode, colorWheelPosition);
      }
    }

    void sequenceEnd(byte sequenceMode)
    {
      sequenceEnd();
      //randomMode = sequenceMode;
    }

    unsigned long getMs()
    {
      #if defined(LED_NEOPIXEL)
      return (unsigned long)(millis() * 1.25);
      #else
      return millis();
      #endif
    }
};
#endif