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
    Playback *playback;
    StripHandler *strip;
    Animation *animation;
    Animation **animations;
    int currentAnimationIndex = 0;

    Animations(Playback *playback, StripHandler *stripHandler)
    {
      this->playback = playback;
      this->strip = stripHandler;
      this->animation = new CylonAnimation(playback, stripHandler, 100);
      
      animations = new Animation*[2];
      animations[0] = new CylonAnimation(playback, stripHandler, 100); 
      animations[1] = new FireboltAnimation(playback, stripHandler, 60);

      setNewAnimation(1);
    }

    void update()
    {
      if (playback->isSequenceEnd)
      {
        animations[currentAnimationIndex]->onSequenceStart();
      }

      animations[currentAnimationIndex]->update();
    }

    void setNewAnimation(int animationIndex)
    {
      currentAnimationIndex = animationIndex;
      animations[currentAnimationIndex]->onStart();
    }
};

#endif
