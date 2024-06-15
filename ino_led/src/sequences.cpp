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

// Linear easing (no easing)
float linearEase(float t) {
    return t;
}

// Quadratic easing in - accelerating from zero velocity
float quadraticEaseIn(float t) {
    return t * t;
}

// Quadratic easing out - decelerating to zero velocity
float quadraticEaseOut(float t) {
    return t * (2 - t);
}

// Quadratic easing in/out - acceleration until halfway, then deceleration
float quadraticEaseInOut(float t) {
    return t < 0.5 ? 2 * t * t : -1 + (4 - 2 * t) * t;
}

// Cubic easing in - accelerating from zero velocity
float cubicEaseIn(float t) {
    return t * t * t;
}

// Cubic easing out - decelerating to zero velocity
float cubicEaseOut(float t) {
    return (--t) * t * t + 1;
}

// Cubic easing in/out - acceleration until halfway, then deceleration
float cubicEaseInOut(float t) {
    return t < 0.5 ? 4 * t * t * t : (t - 1) * (2 * t - 2) * (2 * t - 2) + 1;
}

// Quartic easing in - accelerating from zero velocity
float quarticEaseIn(float t) {
    return t * t * t * t;
}

// Quartic easing out - decelerating to zero velocity
float quarticEaseOut(float t) {
    return 1 - (--t) * t * t * t;
}

// Quartic easing in/out - acceleration until halfway, then deceleration
float quarticEaseInOut(float t) {
    return t < 0.5 ? 8 * t * t * t * t : 1 - 8 * (--t) * t * t * t;
}

// Quintic easing in - accelerating from zero velocity
float quinticEaseIn(float t) {
    return t * t * t * t * t;
}

// Quintic easing out - decelerating to zero velocity
float quinticEaseOut(float t) {
    return 1 + (--t) * t * t * t * t;
}

// Quintic easing in/out - acceleration until halfway, then deceleration
float quinticEaseInOut(float t) {
    return t < 0.5 ? 16 * t * t * t * t * t : 1 + 16 * (--t) * t * t * t * t;
}

// Sine easing in - accelerating from zero velocity
float sineEaseIn(float t) {
    return 1 - cos((t * M_PI) / 2);
}

// Sine easing out - decelerating to zero velocity
float sineEaseOut(float t) {
    return sin((t * M_PI) / 2);
}

// Sine easing in/out - acceleration until halfway, then deceleration
float sineEaseInOut(float t) {
    return -(cos(M_PI * t) - 1) / 2;
}

// Exponential easing in - accelerating from zero velocity
float exponentialEaseIn(float t) {
    return t == 0 ? 0 : pow(2, 10 * (t - 1));
}

// Exponential easing out - decelerating to zero velocity
float exponentialEaseOut(float t) {
    return t == 1 ? 1 : 1 - pow(2, -10 * t);
}

// Exponential easing in/out - acceleration until halfway, then deceleration
float exponentialEaseInOut(float t) {
    if (t == 0 || t == 1) return t;
    return t < 0.5 ? pow(2, 20 * t - 10) / 2 : (2 - pow(2, -20 * t + 10)) / 2;
}

// Circular easing in - accelerating from zero velocity
float circularEaseIn(float t) {
    return 1 - sqrt(1 - t * t);
}

// Circular easing out - decelerating to zero velocity
float circularEaseOut(float t) {
    return sqrt(1 - (--t) * t);
}

// Circular easing in/out - acceleration until halfway, then deceleration
float circularEaseInOut(float t) {
    return t < 0.5 ? (1 - sqrt(1 - 4 * t * t)) / 2 : (sqrt(1 - (--t) * (2 * t - 2)) + 1) / 2;
}

// Elastic easing in - accelerating from zero velocity with elastic effect
float elasticEaseIn(float t) {
    return sin(13 * M_PI_2 * t) * pow(2, 10 * (t - 1));
}

// Elastic easing out - decelerating to zero velocity with elastic effect
float elasticEaseOut(float t) {
    return sin(-13 * M_PI_2 * (t + 1)) * pow(2, -10 * t) + 1;
}

// Elastic easing in/out - acceleration until halfway, then deceleration with elastic effect
float elasticEaseInOut(float t) {
    return t < 0.5
        ? 0.5 * sin(13 * M_PI_2 * (2 * t)) * pow(2, 10 * (2 * t - 1))
        : 0.5 * (sin(-13 * M_PI_2 * (2 * t - 1 + 1)) * pow(2, -10 * (2 * t - 1)) + 2);
}

// Helper function for bounce easing
float bounceEaseOutHelper(float t) {
    if (t < (1 / 2.75)) {
        return 7.5625 * t * t;
    } else if (t < (2 / 2.75)) {
        t -= (1.5 / 2.75);
        return 7.5625 * t * t + 0.75;
    } else if (t < (2.5 / 2.75)) {
        t -= (2.25 / 2.75);
        return 7.5625 * t * t + 0.9375;
    } else {
        t -= (2.625 / 2.75);
        return 7.5625 * t * t + 0.984375;
    }
}

// Bounce easing in - accelerating from zero velocity with bounce effect
float bounceEaseIn(float t) {
    return 1 - bounceEaseOutHelper(1 - t);
}

// Bounce easing out - decelerating to zero velocity with bounce effect
float bounceEaseOut(float t) {
    return bounceEaseOutHelper(t);
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