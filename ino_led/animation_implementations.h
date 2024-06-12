#ifndef ANIMATION_IMPLEMENTATIONS_H
#define ANIMATION_IMPLEMENTATIONS_H

#include <Arduino.h>
#include "debug.h"
#include "animation.h"
#include "sequences.h"
#include "colors.h"
#include "playback.h"


class SineWaveAnimation : public Animation
{
  public:
    float length;

    SineWaveAnimation(Playback *playback, StripHandler *strip, float length) : Animation(playback, strip)
    {
      this->length = length;
    }

    float getIndexFactor(int i)
    {
      return 1.0f * i / playback->pixelCount;
    }

    void update()
    {
      if (playback->updateStepTime(5, true)) return;
      
      for (int i = 0; i < playback->pixelCount; i++)
      {
        float c = 0;
        float indexFactor = getIndexFactor(i);
        float sine1 = sin((playback->normalizedStepTime + indexFactor) * length * 1.0 * PI + playback->animationPlayCount);
        float sine3 = sin((-playback->normalizedStepTime + indexFactor) * length * 1.7 * PI + playback->animationPlayCount);
        float sine2 = sin((playback->normalizedStepTime + indexFactor) * length * 2.5 * PI + playback->animationPlayCount);
        float sine4 = sin((-playback->normalizedStepTime + indexFactor) * length * 5.3 * PI + playback->animationPlayCount);

        c += sine1 + sine2 + sine3 + sine4;
        c = c > 0 ? 1 : 0;
        strip->setPixelColor(i, c);
      }
    }
};



class PulseAnimation : public Animation
{
  public:
    float length;

    PulseAnimation(Playback *playback, StripHandler *strip, int length) : Animation(playback, strip)
    {
      this->length = length;
    }

    void update()
    {
      if (playback->updateStepTime(3.0f, true)) return;

      const float f = 0.8;
      float c = playback->normalizedStepTime;
      c = sin(c * PI);
      c = map(c, f);

      c *= map(sin(playback->normalizedStepTime * PI * 2), f);
      c *= map(sin(playback->normalizedStepTime * PI * 4), f);

      strip->setColorToAll(c);
    }

    float map(float v, float f)
    {
      v = v * 0.5 + 0.5;
      return (1 - f) + v * f;
    }
};


class PingPongAnimation : public Animation
{
  public:
    float length;

    PingPongAnimation(Playback *playback, StripHandler *strip, int length) : Animation(playback, strip)
    {
      this->length = length;
    }

    void update()
    {
      if (playback->updateStepTime(1.0f, 2)) return;

      for (int i = 0; i < playback->pixelCount; i++)
      {
        int ii = (i + playback->ledIndex) % playback->pixelCount;
        int segmentCount = playback->pixelCount / length;
        int o = ii / segmentCount;
        float c = i > playback->ledIndex ? 1.0 : 0.0;

        strip->setPixelColor(i, c);
      }
    }
};


class GrowAnimation : public Animation
{
  public:
    float duration;
    GrowAnimation(Playback *playback, StripHandler *strip, float duration) : Animation(playback, strip)
    {
      this->duration = duration;
    }

    void update()
    {
      if (playback->updateStepTime(duration, 2, false)) return;
      
      byte tailLength = 30;
      bool isUp = playback->sequenceStep % 2 == 0;
      if (isUp)
      {
        playback->ledIndex = easeIn(playback->normalizedStepTime) * playback->pixelCount;
      }
      else
      {
        playback->ledIndex = easeOut(playback->normalizedStepTime) * playback->pixelCount;
      }
      
      for (int i = 0; i < playback->pixelCount; i++)
      {
        int ii = (i + playback->ledIndex) % playback->pixelCount;
        float c;
        
        if (isUp)
        { 
          c = 1 - inverseLerp(playback->ledIndex, playback->ledIndex + tailLength, i); 
        }
        else
        {
          int inv = playback->pixelCount - playback->ledIndex;
          c = 1 - inverseLerp(inv, inv + tailLength, i); 
        }
        
        strip->setPixelColor(i, c);
      }
    }
};

class SparksAnimation : public Animation
{
  public:

    SparksAnimation(Playback *playback, StripHandler *strip) : Animation(playback, strip)
    {
    }

    void update()
    {
      if (playback->updateStepTime(1.5f, true)) return;

      float t = easeIn(playback->normalizedStepTime);
      const byte speed = 4;

      for (int i = 0; i < playback->pixelCount; i++)
      {
        byte value = strip->stripValues[i];
        value -= speed;
        strip->stripValues[i] = value;
      
        float brightness = (1 - t) * value / 255;
        byte c = i;
        strip->setPixelColor(i, wheel(c, brightness));
      }
    }

    void onSequenceStart()
    {
      strip->generateRandomStripValues();
    }
};

class DropsTimeAnimation : public Animation
{
  public:
    int length;

    DropsTimeAnimation(Playback *playback, StripHandler *strip, int length) : Animation(playback, strip)
    {
      this->length = length;
    }

    void update()
    {
      if (playback->updateStepTime(4.0f, true)) return;

      for (int i = 0; i < playback->pixelCount; i++)
      {
        int ii = (i + playback->ledIndex) % playback->pixelCount;
        int segmentCount = playback->pixelCount / length;
        float c = 1.0 * (i % length) / length;
        strip->setPixelColor(ii, c);
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