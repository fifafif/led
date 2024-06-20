#ifndef ANIMATION_IMPLEMENTATIONS_H
#define ANIMATION_IMPLEMENTATIONS_H

#include <Arduino.h>
#include "debug.h"
#include "animation.h"
#include "sequences.h"
#include "colors.h"
#include "playback.h"

class RandomSparksOverdriveAnimatinos : public Animation
{
  public:
    RandomSparksOverdriveAnimatinos(Playback *playback, StripHandler *strip) : Animation(playback, strip)
    {
    }

    void update()
    {
      if (playback->updateStepTime(1.5f)) return;
      
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

class FireworksOverdriveAnimation : public Animation
{  
  public:
    const byte stepCount = 3;

    FireworksOverdriveAnimation(Playback *playback, StripHandler *strip) : Animation(playback, strip)
    {
    }

    void update()
    {
      switch (playback->sequenceStep)
      {
        case 0: fireworksStep1(); break;
        case 1: fireworksStep2(); break;
        case 2: fireworksStep3(); break;
      }
    }

    void fireworksStep1()
    {
      if (playback->updateStepTime(1.0f, stepCount)) return;
      
      playback->ledIndex = sineEaseIn(playback->normalizedStepTime) * playback->pixelCount;

      for (int i = 0; i < playback->pixelCount; i++)
      {
        float c = inverseLerp(playback->ledIndex - 60, playback->ledIndex, i); 
        if (i > playback->ledIndex)
        {
          c = 0;
        }
        strip->setPixelColor(i, clamp01(c));
      }
    }

    void fireworksStep2()
    {
      if (playback->updateStepTime(0.15, stepCount)) return;
      
      playback->ledIndex = playback->pixelCount - easeOut(playback->normalizedStepTime) * playback->pixelCount;

      for (int i = 0; i < playback->pixelCount; i++)
      {
        float c = inverseLerp(playback->ledIndex - 30, playback->ledIndex, i); 
        strip->setPixelColor(i, c);
      }
    }

    void fireworksStep3()
    {
      if (playback->updateStepTime(2, stepCount)) return;

      float exploTime = playback->normalizedStepTime * 4;
      float exploFactor = easeOut(1 - clamp01(exploTime));

      for (int i = 0; i < playback->pixelCount; i++)
      {
        byte value = strip->stripValues[i];
        value -= 10;
        strip->stripValues[i] = value;
      
        float c = 1.0 * value / 255;
        c *= easeIn(1 - playback->normalizedStepTime);
        c = lerpFloat(c, 1.0, exploFactor);

        strip->setPixelColor(i, c);
      }
    }

    void onSequenceStart()
    {
      strip->generateRandomStripValues();
    }
};

class ChargeOverdriveAnimation : public Animation
{
  public:
    int length;
    const byte seqCount = 5;

    ChargeOverdriveAnimation(Playback *playback, StripHandler *strip, int length) : Animation(playback, strip)
    {
      this->length = length;
    }

    void update()
    {
      if (playback->sequenceStep < seqCount - 1) {
        chargeStep1(length);
      } else if (playback->sequenceStep < seqCount) {
        chargeStep2(length); 
      } else {
        chargeStep3();
      }
    }

    void chargeStep3()
    {
      if (playback->updateStepTime(0.5f, true)) return;
      
      playback->ledIndex = easeOut(playback->normalizedStepTime) * playback->pixelCount;
      
      for (int i = 0; i < playback->pixelCount; i++)
      {
        float c = i > playback->ledIndex ? 1 : 0;
        strip->setPixelColor(i, c);
      }
    }

    void chargeStep2(int fillSize)
    {
      if (playback->updateStepTime(0.3f, seqCount)) return;
      
      int fillIndex = playback->sequenceStep * fillSize;
      playback->ledIndex = easeOut(playback->normalizedStepTime) * playback->pixelCount;
      playback->ledIndex = map(playback->ledIndex, 0, playback->pixelCount, fillIndex, playback->pixelCount);

      for (int i = 0; i < playback->pixelCount; i++)
      {
        float c = 1;
        if (i < fillIndex)
        {
          c = 1;
        }
        else
        {
          if (i > playback->ledIndex)
          {
            c = 0;
          }
        }
        strip->setPixelColor(i, clamp01(c));
      }
    }

    void chargeStep1(int fillSize)
    {
      int fillIndex = playback->sequenceStep * fillSize;
      float duration = 1.0f * (1 - 1.0f * fillIndex / playback->pixelCount);
      if (duration < 0.1f)
      {
        duration = 0.1f;
      }

      if (playback->updateStepTime(duration, seqCount)) return;
      
      playback->ledIndex = sineEaseIn(playback->normalizedStepTime) * playback->pixelCount;
      playback->ledIndex = playback->pixelCount - playback->ledIndex;
      playback->ledIndex = map(playback->ledIndex, 0, playback->pixelCount, fillIndex, playback->pixelCount);

      if (playback->sequenceStep > 0)
      {
        float bounceTime = playback->normalizedStepTime * 1.5f;
        float bounceIndex = 0;
        if (bounceTime < 1)
        {
          bounceIndex = sin(PI * easeOut(bounceTime)) * fillSize * 1.5f;
          if (bounceTime > 0.5f && bounceIndex < fillSize)
          {
            bounceIndex = fillSize;
          } 
        }
        else
        {
          bounceIndex = fillSize;
        }

        fillIndex = fillSize * (playback->sequenceStep - 1) + bounceIndex;
      }

      for (int i = 0; i < playback->pixelCount; i++)
      {
        float c = 1;
        if (i < fillIndex)
        {
          c = 1;
        }
        else
        {
          c = 1 - inverseLerp(playback->ledIndex, playback->ledIndex + fillSize, i); 
          if (i < playback->ledIndex)
          {
            c = 0;
          }
        }
        strip->setPixelColor(i, clamp01(c));
      }
    }
};

class MovingFirebolsAnimation : public Animation
{
  public:
    int length;
    float fadeSpeed;
    float *starPositions;
    float *starSpeeds;
    int *starWidths;
    const int STAR_COUNT = 10;
    const int MIN_SPEED = 20;
    const int MAX_SPEED = 200;
    const int MIN_WIDTH = 10;
    const int MAX_WIDTH = 50;

    MovingFirebolsAnimation(Playback *playback, StripHandler *strip, int length) : Animation(playback, strip)
    {
      this->length = length;
      fadeSpeed = 1.0f;
      starPositions = new float[STAR_COUNT];
      starSpeeds = new float[STAR_COUNT];
      starWidths = new int[STAR_COUNT];

      for (int i = 0; i < STAR_COUNT; i++)
      {
        starPositions[i] = random(playback->pixelCount);
        starWidths[i] = (int)random(MIN_WIDTH, MAX_WIDTH);
      }
    }

    void update()
    {
      if (playback->updateStepTime(4.0f)) return;

      strip->clearColor();
      strip->clearRandomStripValues();

      float f = sin(playback->normalizedStepTime * 70);
      f += sin(playback->normalizedStepTime * 40);
      f = f * 0.25 + 0.5;
      f = f * 0.2 + 0.4;
      // Serial.println(f);

      for (int i = 0; i < strip->pixelCount; i++)
      {
        f += random(-0.1, 0.1); 
        strip->setValue(i, f);
      }

      float time = playback->deltaTime * (easeOut(playback->normalizedStepTime) * 0.7f + 0.3f);
      for (int i = 0; i < STAR_COUNT; i++)
      {
        float position = starPositions[i];
        position += (int)round(starSpeeds[i] * time);
        if (position > playback->pixelCount + starWidths[i])
        {
          starSpeeds[i] = random(MIN_SPEED, MAX_SPEED);
          position = -starWidths[i];
        }

        starPositions[i] = position;

        drawFirebolt((int)round(position), starWidths[i]);
      }
    }

    void drawStar(int index, int length)
    {
        int start = index - length;
        int end = index + length;

        for (int i = start; i < end; i++)
        {
          if (i < 0 || i > playback->pixelCount) continue;

          float f = inverseLerp(start, end, i);
          float c = sin((f * f * f * f) * PI);

          strip->addValue(i, c);
        }
    }

    void drawFirebolt(int index, int length)
    {
        int start = index - length;
        int end = index + length / 5;
        
        for (int i = index; i < end; i++)
        {
          if (i < 0 || i > playback->pixelCount) continue;

          strip->addValue(i, 1);
        }

        for (int i = start; i < index; i++)
        {
          if (i < 0 || i > playback->pixelCount) continue;

          float c = easeIn(inverseLerp(start, index, i));

          strip->addValue(i, c * .8f);
        }
    }


    void onStart()
    {
      strip->clearRandomStripValues();
      strip->clearColor();
      generateRandomSpeed();
    }

    void onSequenceStart()
    {
      generateRandomSpeed();
    }

    void generateRandomSpeed()
    {
      for (int i = 0; i < STAR_COUNT; i++)
      {
        starSpeeds[i] = random(MIN_SPEED, MAX_SPEED);
      }
    }
};

class MovingStarsAnimation : public Animation
{
  public:
    int length;
    float fadeSpeed;
    float *starPositions;
    float *starSpeeds;
    int *starWidths;
    const int STAR_COUNT = 10;
    const float MIN_SPEED = 20;
    const float MAX_SPEED = 60;

    MovingStarsAnimation(Playback *playback, StripHandler *strip, int length) : Animation(playback, strip)
    {
      this->length = length;
      fadeSpeed = 1.0f;
      starPositions = new float[STAR_COUNT];
      starSpeeds = new float[STAR_COUNT];
      starWidths = new int[STAR_COUNT];

      for (int i = 0; i < STAR_COUNT; i++)
      {
        starPositions[i] = random(playback->pixelCount);
        starWidths[i] = (int)random(6, 30);
      }
    }

    void update()
    {
      if (playback->updateStepTime(4.0f)) return;

      // byte fade = round(255 * fadeSpeed * playback->deltaTime);
      strip->clearColor();
      strip->clearRandomStripValues();

      for (int i = 0; i < STAR_COUNT; i++)
      {
        float position = starPositions[i];

        //logNumbers(position, playback->deltaTime);
        position += (int)round(starSpeeds[i] * playback->deltaTime);
        if (position > playback->pixelCount)
        {
          position -= playback->pixelCount;
        }
        else if (position < 0)
        {
          position += playback->pixelCount;
        }

        starPositions[i] = position;

       drawStar((int)round(position), starWidths[i]);
      }
    }

    void drawStar(int index, int length)
    {
        int start = index - length;
        int end = index + length;

        for (int i = start; i < end; i++)
        {
          float c;
          if (i < index)
          {
            c = 1.0f * (i - start) / length;
          }
          else if (i > index)
          {
            c = 1.0f * (end - i) / length;
          }
          else
          {
            c = 1;
          }

          int ii = i;
          if (ii < 0)
          {
            ii += playback->pixelCount;
          }
          else if (ii >= playback->pixelCount)
          {
            ii -= playback->pixelCount;
          }

          // Serial.print(ii);
          // Serial.print(",");
          c = max(strip->stripValues[ii] / 255.0f, c); 
          strip->stripValues[ii] = (byte)(c * 255);
          strip->setPixelColor(ii, c);
        }
    }

    void onStart()
    {
      strip->clearRandomStripValues();
      strip->clearColor();
      generateRandomSpeed();
    }

    void onSequenceStart()
    {
      generateRandomSpeed();
    }

    void generateRandomSpeed()
    {
        // Serial.print("speeds ");
      for (int i = 0; i < STAR_COUNT; i++)
      {
        int speed = random(MIN_SPEED, MAX_SPEED);
        if (random(2) > 0)
        {
          speed = -speed;
        }

        starSpeeds[i] = speed;

        // Serial.println(starSpeeds[i]);
      }
    }
};

class StarsAnimation : public Animation
{
  public:
    int length;
    float timeToNewStar;
    float fadeSpeed;
    float newStarIntervalnterval;

    StarsAnimation(Playback *playback, StripHandler *strip, int length) : Animation(playback, strip)
    {
      this->length = length;
      fadeSpeed = 1.0f;
      newStarIntervalnterval = 0.02f;
    }

    void update()
    {
      if (playback->updateStepTime(2.0f)) return;

      byte fade = round(255 * fadeSpeed * playback->deltaTime);
      
      for (int i = 0; i < playback->pixelCount; i++)
      {
        byte value = strip->stripValues[i];
        if (value < fade)
        {
          value = 0;
        }
        else
        {
          value -= fade;
          if (value < 0)
          {
            value = 0;
          }
        }

        strip->stripValues[i] = value;
        strip->setPixelColor(i, value / 255.0f);
      }

      timeToNewStar -= playback->deltaTime;
      if (timeToNewStar <= 0)
      {
        timeToNewStar += newStarIntervalnterval;

        int index = random(playback->pixelCount);
        int start = max(0, index - length);
        int end = min(playback->pixelCount, index + length);

        for (int i = start; i < end; i++)
        {
          float c;
          if (i < index)
          {
            c = 1.0f * (i - start) / length;
          }
          else if (i > index)
          {
            c = 1.0f * (end - i) / length;
          }
          else
          {
            c = 1;
          }

          c = max(strip->stripValues[i] / 255.0f, c); 
          strip->stripValues[i] = (byte)(c * 255);
          strip->setPixelColor(i, c);
        }
      }
    }

    void onStart()
    {
      strip->clearRandomStripValues();
      strip->clearColor();
    }
};

class SegmentFillAnimation : public Animation
{
  public:
    int segmentCount;
    float *segmentValues;
    int currentSegmentIndex = 0;
    int segmentWidth;
    int *segmentIndices;

    SegmentFillAnimation(Playback *playback, StripHandler *strip, int segmentCount) : Animation(playback, strip)
    {
      this->segmentCount = segmentCount;
      segmentWidth = (int)round(1.0 * playback->pixelCount / segmentCount);
      segmentValues = new float[segmentWidth];
      segmentIndices = new int[segmentCount];

      for (int i = 0; i < segmentCount; i++)
      {
        segmentIndices[i] = i;
      }
    }

    void update()
    {
      if (playback->updateStepTime(0.75f, segmentCount)) return;

      int start = currentSegmentIndex * segmentWidth;
      int end = start + segmentWidth;

      // logNumbers("Start end: ", start, end);
      
      bool isReverse = playback->sequenceStep % 2 == 0;
      int ledIndex = start + (int)round(bounceEaseOut(playback->normalizedStepTime) * segmentWidth);

      for (int i = start, j = 0; i < end && i < playback->pixelCount; i++, j++)
      {
        float c = i > ledIndex ? 0 : 1;

        if (isReverse)
        {
          strip->setPixelColor(end - j - 1, c);
        }
        else
        {
          strip->setPixelColor(i, c);
        }
      }
    }

    void onBeat()
    {
      playback->stepTimeEnd();
    }

    void onSequenceStart()
    {
      currentSegmentIndex = 0;

      strip->setColorToAll((uint32_t)0);

      for (int i = segmentCount - 1; i > 0; i--) 
      {
        int j = random(i + 1);
        int temp = segmentIndices[i];
        segmentIndices[i] = segmentIndices[j];
        segmentIndices[j] = temp;
      }

      // printArray(segmentIndices, segmentCount);
    }

    void onStepStart()
    {
      if (playback->sequenceStep > 0)
      {
        int start = currentSegmentIndex * segmentWidth;
        int end = start + segmentWidth;

        for (int i = start, j = 0; i < end && i < playback->pixelCount; i++, j++)
        {
            strip->setPixelColor(i, 1.0f);
        }
      }

      currentSegmentIndex = segmentIndices[playback->sequenceStep];
    }
};

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
      if (playback->updateStepTime(5)) return;
      
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
      if (playback->updateStepTime(3.0f)) return;

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
      if (playback->updateStepTime(duration, 2)) return;
      
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
      if (playback->updateStepTime(1)) return;

      float t = quadraticEaseIn(playback->normalizedStepTime);
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

    void onBeat()
    {
      // FIX: COntinue in sequence time
      playback->startStepTime();
    }
};

class DropsTimeAnimation : public Animation
{
  public:
    int length;

    DropsTimeAnimation(Playback *playback, StripHandler *strip, int length) : Animation(playback, strip)
    {
      this->length = length;
      beatDuration = 0.5f;
    }

    void update()
    {
      if (playback->updateStepTime(4.0f)) return;

      float t = clamp01(getElapsedBeatFactor());

      for (int i = 0; i < playback->pixelCount; i++)
      {
        int ii = (i + playback->ledIndex) % playback->pixelCount;
        float c = 1.0 * (i % length) / length;

        c = easeIn(c);

        strip->stripValues[ii] = c * 255;
        strip->setPixelColor(ii, c);
      }

      centerGlowBeat(200, 0.5f, 1.0f);
      // centerGlowBeat(100, 0.4f, 1.0f, true);
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

    void onBeat()
    {
      playback->moveTime(-0.1f);
    }
};

class FireboltAnimation : public Animation
{
  public:
    const float FADE_SPEED = 0.75f;
    const float BEAT_MAX_VALUE = 0.75f;
    
    int length;
    float beatValue = 0;

    FireboltAnimation(Playback *playback, StripHandler *strip, int length) : Animation(playback, strip)
    {
      this->length = length;
    }

    void update()
    {
      if (playback->updateStepTime(2.0f)) return;

      playback->ledIndex = easeOut(playback->normalizedStepTime) * playback->pixelCount;

      if (beatValue > 0)
      {
        beatValue -= playback->deltaTime * FADE_SPEED;
        if (beatValue < 0)
        {
          beatValue = 0;
        } 
      }

      for (int i = 0; i < playback->pixelCount; i++)
      {
        float c = inverseLerp(playback->ledIndex - length, playback->ledIndex, i);
        c = easeOut(c);
        c += beatValue;

        if (i > playback->ledIndex)
        {
          c = beatValue;
        }
       
        byte value = strip->stripValues[i];
        value -= 10;
        strip->stripValues[i] = value;
        c *= value / 255.0;

        strip->setPixelColor(i, clamp01(c));
      }
    }

    void onSequenceStart()
    {
      log("Firebolt onSequenceStart");
      strip->generateRandomStripValues();
    }

    void onBeat()
    {
      beatValue = BEAT_MAX_VALUE;
    }
};

class CylonAnimation : public Animation
{
  public:
    int length;
    float beatLength;
    const float MAX_BEAT_LENGTH_PERCENT = 50;
    const float BEAT_FADE_DURATION = 0.5f;
    const float BEAT_VALUE = 0.5f;
    float maxBeatLength;

    CylonAnimation(Playback *playback, StripHandler *strip, int length) : Animation(playback, strip)
    {
      this->length = length;
      maxBeatLength = playback->pixelCount * MAX_BEAT_LENGTH_PERCENT * 0.01f;
    }

    void update()
    {
      if (playback->updateStepTime(1.0f, 2)) return;

      bool isReverse = playback->sequenceStep % 2 == 1;
      int ledIndex = easeOut(playback->normalizedStepTime) * playback->pixelCount;
      int currentLength = lerp(length / 2, length, playback->normalizedStepTime);

      if (beatLength > 0)
      {
        beatLength -= playback->deltaTime * maxBeatLength / BEAT_FADE_DURATION;
        if (beatLength < 0)
        {
          beatLength = 0;
        }
      }

      for (int i = 0; i < playback->pixelCount; i++)
      {
        float c = inverseLerp(ledIndex - currentLength, ledIndex, i);
        c = easeIn(c);
        if (i > ledIndex)
        {
          c = 0;
        }

        if (isReverse) {
          strip->setPixelColor(i, c);
          strip->stripValues[i] = c * 255;
        } else {
          strip->setPixelColor(playback->pixelCount - i - 1, c);
          strip->stripValues[playback->pixelCount - i - 1] = c * 255;
        }
      }

      centerGlowBeat(beatLength, BEAT_VALUE);
    }

    void onBeat()
    {
      beatLength = maxBeatLength;
    }
};

#endif