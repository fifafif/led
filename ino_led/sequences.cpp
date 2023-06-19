#include "sequences.h"
#include <Arduino.h>

#define PI 3.14159

float easeIn(float t)
{
  return t * t;
}

float easeOut(float t)
{
  return 1 - (1 - t) * (1 - t);
}

float easeOutSine(float t) 
{
    return sin((t * PI) / 2);
}

float easeOutCircle(float t) 
{
    return sqrt(1 - pow(t - 1, 2));
}

float clamp01(float t)
{
  return 
    t < 0 
    ? 0 
    : (t > 1 
      ? 1 
      : t);
}


float lerp(int a, int b, float t)
{
  if (t >= 1) return b; 
  if (t <= 0) return a;
  
  return a + (b - a) * t;
}

float lerp(float a, float b, float t)
{
  if (t >= 1) return b; 
  if (t <= 0) return a;
  
  return a + (b - a) * t;
}

float inverseLerp(int a, int b, float t)
{
  if (b == a) return 0;
  if (t <= a) return 0;
  if (t >= b) return 1;
  
  return (t - a) / (b - a);
}