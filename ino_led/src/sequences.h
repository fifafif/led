#ifndef SEQUENCES_H
#define SEQUENCES_H

enum seqType
{
  WAVE_SIN,
  WAVE_SIN_HALF,
  COUNT
};

float easeIn(float t);
float easeOut(float t);
float linearEase(float t);
float quadraticEaseIn(float t);
float quadraticEaseOut(float t);
float quadraticEaseInOut(float t);
float cubicEaseIn(float t);
float cubicEaseOut(float t);
float cubicEaseInOut(float t);
float quarticEaseIn(float t);
float quarticEaseOut(float t);
float quarticEaseInOut(float t);
float quinticEaseIn(float t);
float quinticEaseOut(float t);
float quinticEaseInOut(float t);
float sineEaseIn(float t);
float sineEaseOut(float t);
float sineEaseInOut(float t);
float exponentialEaseIn(float t);
float exponentialEaseOut(float t);
float exponentialEaseInOut(float t);
float circularEaseIn(float t);
float circularEaseOut(float t);
float circularEaseInOut(float t);
float elasticEaseIn(float t);
float elasticEaseOut(float t);
float elasticEaseInOut(float t);
float bounceEaseIn(float t);
float bounceEaseOut(float t);
float bounceEaseInOut(float t);

float clamp01(float t);
float lerp(int a, int b, float t);
// float lerpFloat(int a, int b, float t);
float lerpFloat(float a, float b, float t);
float lerp(float a, float b, float t);
float inverseLerp(int a, int b, float t);

#endif