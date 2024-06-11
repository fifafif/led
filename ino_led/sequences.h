#ifndef SEQUENCES_H
#define SEQUENCES_H

enum seqType
{
  WAVE_SIN,
  WAVE_SIN_HALF,
  COUNT
};

float easeIn(float t);
float easeInSine(float t);
float easeOut(float t);
float easeOutSine(float t);
float easeOutCircle(float t);
float clamp01(float t);
float lerp(int a, int b, float t);
//float lerp(float a, float b, float t);
float inverseLerp(int a, int b, float t);

#endif