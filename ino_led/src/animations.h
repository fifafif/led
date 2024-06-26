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
    const int ANIMATION_COUNT = 13;
    const int OVERDRIVE_ANIMATION_COUNT = 5;
    const int OVERDRIVE_INDEX_1_LENGTH = 5;
    
  #if defined(PROD)
    const byte switchAutoModeEveryTickCount = 16;
  #else
    const byte switchAutoModeEveryTickCount = 2;
  #endif
  
    const byte slaveColorModeCount = 4;

    Playback *playback;
    StripHandler *strip;
    Animation *animation;
    Animation **animations;
    Animation **overdriveAnimations;
    int **overdriveIndices;
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
      animations[0] = new CylonAnimation(playback, stripHandler, 100, 1.0f); 
      animations[1] = new CylonAnimation(playback, stripHandler, 50, 2.0f); 
      animations[2] = new FireboltAnimation(playback, stripHandler, 60, 2.0f);
      animations[3] = new FireboltAnimation(playback, stripHandler, 40, 4.0f);
      animations[4] = new SegmentAnimation(playback, stripHandler, 0, 50);
      animations[5] = new SegmentAnimation(playback, stripHandler, 1, 50);
      animations[6] = new DropsTimeAnimation(playback, stripHandler, 50);
      // animations[5] = new SparksAnimation(playback, stripHandler);
      animations[7] = new GrowAnimation(playback, stripHandler, 1);
      animations[8] = new PingPongAnimation(playback, stripHandler, 100);
      animations[9] = new PulseAnimation(playback, stripHandler, 100);
      animations[10] = new SineWaveAnimation(playback, stripHandler, 2);
      // animations[10] = new SegmentFillAnimation(playback, stripHandler, 6);
      animations[11] = new StarsAnimation(playback, stripHandler, 6);
      // animations[11] = new MovingStarsAnimation(playback, stripHandler, 10);
      // animations[12] = new MovingFirebolsAnimation(playback, stripHandler, 10);

      overdriveAnimations = new Animation*[OVERDRIVE_ANIMATION_COUNT];
      overdriveAnimations[0] = new ChargeOverdriveAnimation(playback, stripHandler, 20); 
      overdriveAnimations[1] = new FireworksOverdriveAnimation(playback, stripHandler);
      overdriveAnimations[2] = new RandomSparksOverdriveAnimatinos(playback, stripHandler);
      overdriveAnimations[3] = new AlarmOverdriveAnimation(playback, stripHandler);
      overdriveAnimations[4] = new PoliceOverdriveAnimation(playback, stripHandler);

      // int jagged[][3] = { {0,1}, {1,2,3} };
      
      // overdriveIndices = new int*[3];
      // overdriveIndices[0] = new int[2];
      // overdriveIndices[0][0] = 0;
      // overdriveIndices[0][1] = 1;

      // overdriveIndices[1] = new int[2];
      // overdriveIndices[1][0] = 2;
      // overdriveIndices[1][1] = 3;

      // overdriveIndices[2] = new int[1];
      // overdriveIndices[2][0] = 4;


#if !defined(PROD)
      debugAnimationIndex = 11;
      // debugOverdriveAnimationIndex = 4;
      // isOverdrive = true;

      // if (isOverdrive)
      // {
      //   choseRandomOverdrive();
      // }
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
      Serial.print("New animation: ");
      Serial.println(index);
      
      currentAnimationIndex = index;
      animations[index]->onStart();
    }

    void setNewAnimationOveredrive(int index)
    {
      currentOverdriveAnimationIndex = index;
      playback->startStepTime();
      overdriveAnimations[index]->onStart();
    }

    int choseRandomOverdriveIndex(byte index)
    {
      switch(index)
      {
        case 0: {
          byte i = random(0, 2);
          switch (i) {
            case 0: return 0;
            case 1: return 1;
          }
          break;
        }
        case 1: {
          byte i = random(0, 2);
          switch (i) {
            case 0: return 2;
            case 1: return 3;
          }
          break;
        }
        case 2: return 4;
      }
      // TODO: Implement!
      return 0;
      // byte length = sizeof(overdriveIndices[index]) / sizeof(int);
      // long i = random(0, length);
      // return overdriveIndices[index][i];
    }

    void choseRandomOverdrive(byte index)
    {
      if (OVERDRIVE_ANIMATION_COUNT < 1) return;
      
      int i = choseRandomOverdriveIndex(index);
      setNewAnimationOveredrive(i);
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
        newIndex = random(12);
      }

      String msg = "new sequence: ";
      msg += newIndex;
      log(msg);

      setNewAnimation(newIndex);
    }
        
    void startOverdrive(byte index)
    {
      log("start overdrive!");
      // playback->startStepTime();
      isOverdrive = true;

      if (!isSlave)
      {

        choseRandomOverdrive(index);
      }  
    }

    void startOverdriveRandom(byte index)
    {
      Serial.print("start overdrive random: ");
      Serial.println(index);
      // playback->startStepTime();
      isOverdrive = true;

      if (!isSlave)
      {
        choseRandomOverdrive(index);
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
