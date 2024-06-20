#ifndef ANIMATIONS_H
#define ANIMATIONS_H

#include <Arduino.h>
#include "tick_payload.h"
#include "sequences.h"
#include "colors.h"
#include "playback.h"
#include "animation_implementations.h"

class Animations
{
  public:
    const int ANIMATION_COUNT = 14;
    const int OVERDRIVE_ANIMATION_COUNT = 5;
    
  #if defined(PROD)
    const byte switchAutoModeEveryTickCount = 8;
  #else
    const byte switchAutoModeEveryTickCount = 2;
  #endif
  
    const byte slaveColorModeCount = 4;

    Playback *playback;
    StripHandler *strip;
    Animation *animation;
    Animation **animations;
    Animation **overdriveAnimations;
    bool isOverdrive;
    bool isSlave;

    int currentAnimationIndex = 0;
    int currentOverdriveAnimationIndex = 0;
    int debugAnimationIndex = -1;
    int debugOverdriveAnimationIndex = -1;

    // Color
    // DMX = 0, Rnd = 1, Wheel = 2, WheelStep = 3, Serial = 4
    byte colorMode = 1;
    byte colorWheelStep = 255 / 6;
    byte colorStepCount = 6;
    int colorIndex = 0;
    byte slaveColorMode = 0;

    Animations(Playback *playback, StripHandler *stripHandler)
    {
      this->playback = playback;
      this->strip = stripHandler;
      
      animations = new Animation*[ANIMATION_COUNT];
      animations[0] = new CylonAnimation(playback, stripHandler, 100); 
      animations[1] = new FireboltAnimation(playback, stripHandler, 60);
      animations[2] = new SegmentAnimation(playback, stripHandler, 0, 50);
      animations[3] = new SegmentAnimation(playback, stripHandler, 1, 50);
      animations[4] = new DropsTimeAnimation(playback, stripHandler, 50);
      animations[5] = new SparksAnimation(playback, stripHandler);
      animations[6] = new GrowAnimation(playback, stripHandler, 1);
      animations[7] = new PingPongAnimation(playback, stripHandler, 100);
      animations[8] = new PulseAnimation(playback, stripHandler, 100);
      animations[9] = new SineWaveAnimation(playback, stripHandler, 2);
      // animations[10] = new SegmentFillAnimation(playback, stripHandler, 6);
      animations[10] = new StarsAnimation(playback, stripHandler, 6);
      animations[11] = new StarsAnimation(playback, stripHandler, 6);
      animations[12] = new MovingStarsAnimation(playback, stripHandler, 10);
      animations[13] = new MovingFirebolsAnimation(playback, stripHandler, 10);

      overdriveAnimations = new Animation*[OVERDRIVE_ANIMATION_COUNT];
      overdriveAnimations[0] = new ChargeOverdriveAnimation(playback, stripHandler, 20); 
      overdriveAnimations[1] = new FireworksOverdriveAnimation(playback, stripHandler);
      overdriveAnimations[2] = new RandomSparksOverdriveAnimatinos(playback, stripHandler);
      overdriveAnimations[3] = new AlarmOverdriveAnimation(playback, stripHandler);
      overdriveAnimations[4] = new PoliceOverdriveAnimation(playback, stripHandler);

#if !defined(PROD)
      // debugAnimationIndex = 0;
      debugOverdriveAnimationIndex = 4;
      isOverdrive = true;

      if (isOverdrive)
      {
        choseRandomOverdrive();
      }
#endif

      choseRandomAnimation();
    }

    void update()
    {
      if (isOverdrive)
      {
        updateOverrideAnimation();
      }
      else
      {
        updateAnimation();
      }
    }

    void updateAnimation(Animation *animation, bool isOverdrive)
    {
      if (playback->isSequenceEnd)
      {
        if (isOverdrive)
        {
        #if !defined(PROD)
          if (debugOverdriveAnimationIndex < 0)
        #endif
          {
            this->isOverdrive = false;
            choseRandomAnimation();
            animation = animations[currentAnimationIndex];
          }
        }
        else
        {
          if (playback->animationPlayCount % switchAutoModeEveryTickCount == 0)
          {
            choseRandomAnimation();
          }
        }

        animation->onSequenceStart();
        animation->onStepStart();
        changeColor();
      }
      else if (playback->isStepEnd)
      {
        animation->onStepStart();
      }

      animation->update();
    }

    void updateAnimation()
    {
      updateAnimation(animations[currentAnimationIndex], false);
    }

    void updateOverrideAnimation()
    {
      updateAnimation(overdriveAnimations[currentOverdriveAnimationIndex], true);
    }

    void startNewAnimation(int animationIndex)
    {
      playback->sequenceEnd();
      setNewAnimation(animationIndex);
    }

    void setNewAnimation(int index)
    {
      currentAnimationIndex = index;
      animations[index]->onStart();
    }

    void setNewAnimationOveredrive(int index)
    {
      currentOverdriveAnimationIndex = index;
      overdriveAnimations[index]->onStart();
    }

    void choseRandomOverdrive()
    {
      if (OVERDRIVE_ANIMATION_COUNT < 1) return;
      
      if (debugOverdriveAnimationIndex > -1)
      {
        setNewAnimationOveredrive(debugOverdriveAnimationIndex);
        return;
      }

      setNewAnimationOveredrive(random(OVERDRIVE_ANIMATION_COUNT));
    }

    void choseRandomAnimation()
    {
      if (ANIMATION_COUNT <= 1) return;

      if (debugAnimationIndex >= 0)
      {
        setNewAnimation(debugAnimationIndex);
        return;
      }

      int newIndex = currentAnimationIndex;
      while (newIndex == currentAnimationIndex)
      {
        newIndex = random(ANIMATION_COUNT);
      }

      String msg = "new sequence: ";
      msg += newIndex;
      log(msg);

      setNewAnimation(newIndex);
    }
        
    void startOverdrive()
    {
      log("start overdrive!");
      playback->startStepTime();
      isOverdrive = true;

      if (!isSlave)
      {
        choseRandomOverdrive();
      }  
    }

    void changeColor()
    {
      updateColorSeqEnd(colorMode);
    }

    void updateColorSeqEnd(byte colorMode)
    {
      if (colorMode == 3)
      {
        colorIndex += 1;
        if (colorIndex > colorStepCount)
        {
          colorIndex = 0;
        }
        strip->setRGBFromWheel(colorIndex * 255 / colorStepCount);
      }
      else if (colorMode == 1)
      {
        strip->setRGBFromWheel(random(255));
      }
      else if (colorMode == 4)
      {
        //updateSlaveColor(getSerialMessageColor());
      }
    }

    void updateSlaveColor(byte slaveColor)
    {
      switch (slaveColorMode)
      {
        case 1:
          slaveColor += 127;
          break;   
        case 2:
          slaveColor += 30;
          break;
        case 3:
          slaveColor -= 30;
          break;
      }

      strip->setRGBFromWheel(slaveColor);        
    }

    void setAsSlave(bool isSlave)
    {
      this->isSlave = isSlave;
      colorMode = isSlave ? 4 : 1;
    }

    void beat()
    {
      animations[currentAnimationIndex]->beat();
    }
};

#endif
