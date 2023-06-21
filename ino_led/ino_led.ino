//#define DMX_ON
//#define LED_SIM_ONLY

#if defined(ARDUINO_AVR_MEGA2560) || defined(ARDUINO_AVR_MEGA) || defined(ARDUINO_AVR_NANO) || defined(ARDUINO_AVR_UNO)
#define LED_NEOPIXEL
#define LED_ONBOARD LED_BUILTIN
#else
#define LED_ONBOARD 2
#endif

#include <Arduino.h>

#if defined(DMX_ON)

#include <DMXSerial.h>
#include <DMXSerial_avr.h>
#include <DMXSerial_megaavr.h>

#endif

#if !defined(LED_SIM_ONLY)

#if defined(LED_NEOPIXEL)
#include <Adafruit_NeoPixel.h>
#else
#include <LiteLED.h>
#endif

#endif

#include "sequences.h"

#define LED_PIN 4
#define BUTTON_IN 3
#define BEAT_IN 5
#define NUMPIXELS 300
#define CENTERPIX NUMPIXELS / 2
#define SEQ_LENGTH 30
#define DMXSTART 139
#define DMXLENGTH (512)
#define PI 3.14159
#define PI2 2 * PI

#if defined(LED_SIM_ONLY)
uint32_t strip[NUMPIXELS] = {};
#else

#if defined(LED_NEOPIXEL)
Adafruit_NeoPixel strip(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
#else
LiteLED strip(LED_STRIP_WS2812, 0);
#endif

#endif

byte redValue = 255;
byte greenValue = 255;
byte blueValue = 0;

// DMX Values
byte redValueDMX = 255;
byte greenValueDMX = 255;
byte blueValueDMX = 0;
float brightness = 0.1;
byte stroboMode = 0;
byte mode = 255;

int delayTicks = 1;

byte seqCount = 12;
byte randomMode = 0;

// Time
byte speed = 127;
int seqLength = 60;
float acceleration = 3;
byte incrementStep = 1;
int tickCount = 0;
int ledIndex;
float ledIndexFloat = 0;
byte switchAutoModeEveryTickCount = 8;
bool isTickEnd = true;
unsigned long tickStartMs;
float normalizedTickTime;
int ticksSinceLastBeat = 666;
int lastBeatState;

// Color
byte colorMode = 1;
byte colorWheelStep = 255 / 6;
byte colorStepCount = 6;
int colorIndex = 0;

// Sequences
byte stripValues[NUMPIXELS] = {};
int fill = 0;
int sequenceStep;

// Overdrive
bool isOverdrive;
int lastOverdriveButtonState;
int remainingOverdriveLoopCount;
float overdriveAcceleration = 9;
const int overdriveLoopCount = 3;

void setup () 
{
  pinMode(LED_ONBOARD, OUTPUT);
  pinMode(BUTTON_IN, INPUT);
  pinMode(BEAT_IN, INPUT);
  
  randomSeed(analogRead(0) + analogRead(1));

#if defined(DMX_ON)
  DMXSerial.init(DMXProbe);
  DMXSerial.maxChannel(DMXLENGTH);
#else
  Serial.begin(115200);
  Serial.write("LED Start");
#endif   
  
#if !defined(LED_SIM_ONLY)

#if defined(LED_NEOPIXEL)
  strip.begin();
#else
  strip.brightness(255);
  strip.begin(LED_PIN, NUMPIXELS);
#endif

#else
  brightness = 1;
#endif
}

void loop() 
{
  readDMX();

  colorMode = 1;

  readBeat();
  readOverrideButton();
  
  updateColorTick(colorMode);
  /*
  if (isOverdrive)
  {
    updateSeqOverdrive();
  }
  else
  {
    updateSeq();
  }*/
  
  //testSequence();
  //firebolt();
  dropsTime(50);
  //fireworks();
  //centerWave(60);
 
#if defined(LED_SIM_ONLY)
  logStrip();
  delay(10);
#else
  strip.show();
#endif

  //delay(delayTicks);
}

void readBeat()
{
  ticksSinceLastBeat += 1;

  int beatState = digitalRead(BEAT_IN);
  digitalWrite(LED_ONBOARD, beatState);
  //Serial.println("beat " + beatState);
  if (beatState == HIGH
      && beatState != lastBeatState)
  {
    beat();
  }

  lastBeatState = beatState;
}

void beat()
{
  log("beat!");
  ticksSinceLastBeat = 0;
  tickEnd();
}

void dropsTime(int segmentLength)
{
  tickStepTime(10.0f);

  for (int i = 0; i < NUMPIXELS; i++)
  {
    int ii = (i + ledIndex) % NUMPIXELS;
    int segmentCount = NUMPIXELS / segmentLength;
    float c = 1.0 * (i % segmentLength) / segmentLength;
    setPixelColor(ii, c);
  }
}

void firebolt()
{
  /*tickStepTime(1.0);
  if (normalizedTickTime >= 1)
  {
    tickStepTimeEnd();
    //return;
  }
  */
  ledIndex = easeOut(normalizedTickTime) * NUMPIXELS;

  for (int i = 0; i < NUMPIXELS; i++)
  {
    float c = inverseLerp(ledIndex - 80, ledIndex, i); 
    if (i > ledIndex)
    {
      c = 0;
    }
    else
    {
      //c *= random(100) * 0.01;
    }

    setPixelColor(i, clamp01(c));
  }
}

void fireworks()
{
  switch (sequenceStep)
  {
    case 0: fireworksStep1(); break;
    case 1: fireworksStep2(); break;
    case 2: fireworksStep3(); break;
  }
}

void fireworksStep1()
{
  if (tickStepTime(1.0f))
  {
    for (int i = 0; i < NUMPIXELS; i++)
    {
      stripValues[i] = random(255);
    }
    return;
  }
  
  ledIndex = easeOutSine(normalizedTickTime) * NUMPIXELS;

  for (int i = 0; i < NUMPIXELS; i++)
  {
    float c = inverseLerp(ledIndex - 60, ledIndex, i); 
    if (i > ledIndex)
    {
      c = 0;
    }
    setPixelColor(i, clamp01(c));
  }
}

void fireworksStep2()
{
  if (tickStepTime(0.15))
  {
    return;
  }
  
  ledIndex = NUMPIXELS - easeOut(normalizedTickTime) * NUMPIXELS;

  for (int i = 0; i < NUMPIXELS; i++)
  {
    float c = inverseLerp(ledIndex - 30, ledIndex, i); 
    setPixelColor(i, c);
  }
}

void fireworksStep3()
{
  if (tickStepTime(2))
  {
    sequenceStep = 0;
    return;
  }

  float exploTime = normalizedTickTime * 4;
  float exploFactor = easeOut(1 - clamp01(exploTime));

  for (int i = 0; i < NUMPIXELS; i++)
  {
    byte value = stripValues[i];
    value -= 10;
    stripValues[i] = value;
  
    float c = 1.0 * value / 255;
    c *= easeIn(1 - normalizedTickTime);

    c = lerp(c, 1.0, exploFactor);

    setPixelColor(i, c);
  }
}

void readOverrideButton()
{
  int buttonState = digitalRead(BUTTON_IN);
  if (buttonState == HIGH
      && buttonState != lastOverdriveButtonState)
  {
    startOverdrive();  
  }

  lastOverdriveButtonState = buttonState;

/*
  if (buttonState == HIGH) {
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }*/
}

void startOverdrive()
{
  log("start overdrive!");
  isOverdrive = true;
  remainingOverdriveLoopCount = overdriveLoopCount;
  choseRandomSequence();
  tickEnd();
}

void updateSeqOverdrive()
{
  int s = 0;
  
  switch(s)
  {
    case 0: fireworks(); break;
    //case 2: dropsTime(30); break;
  }
}

void updateSeq()
{
  int s = int(seqCount) * 255 / mode;
  if (s == seqCount)
  {
    s = randomMode;
  }
  s = 12;
  
  switch(s)
  {
    case 0: solid(); break;
    case 1: pulse(40); break;
    case 2: stroboFade(10); break;
    case 3: pong(); break;
    case 4: drops(20); break;
    case 5: pingPong(20); break;
    case 6: wavesSeq(); break;
    case 7: wavesHalfSeq(); break;
    case 8: wavesSqrtSeq(); break;
    case 9: wavesHardSeq(); break;
    case 10: rainbow(60); break;  
    case 11: rainbow(20); break;
    case 12: dropsTime(30); break;
  }
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
    rgbFromWheel(colorIndex * 255 / colorStepCount);
  }
  else if (colorMode == 1)
  {
    rgbFromWheel(random(255));
  }
}

// DMX = 0, Rnd = 1, Wheel = 2, WheelStep = 3

void updateColorTick(byte colorMode)
{
  if (colorMode == 0)
  {
    redValue = redValueDMX;
    greenValue = greenValueDMX;
    blueValue = blueValueDMX;
  }
  else if (colorMode == 2)
  {
    rgbFromWheel(ledIndex);
  }
}

void wavesSeq()
{
  float s[seqLength];
  for (int i = 0; i < seqLength; ++i)
  {
    float v = sin(PI * float(i) / seqLength);
    v = v * 1.2 - 0.2;
    s[i] = v > 0 ? v : 0;
  }

  updateSeq(s);
}

void wavesHalfSeq()
{
  float s[SEQ_LENGTH];
  for (int i = 0; i < SEQ_LENGTH; ++i)
  {
    float v = sin(0.5 * PI * float(i) / SEQ_LENGTH);
    v = v * 1.2 - 0.2;
    s[i] = v > 0 ? v : 0;
  }

  updateSeq(s);
}

void wavesSqrtSeq()
{
  float s[SEQ_LENGTH];
  for (int i = 0; i < SEQ_LENGTH; ++i)
  {
    //float v = float(i) / SEQ_LENGTH);
    //v = v * 1.2 - 0.2;
    float x = float(i) / SEQ_LENGTH;
    s[i] = x * x;
  }

  updateSeq(s);
}

void wavesHardSeq()
{
  float s[seqLength];
  for (int i = 0; i < seqLength; ++i)
  {
    s[i] = i > seqLength / 2 ? 1 : 0;
  }

  updateSeq(s);
}

void updateSeq(float seqValues[])
{
  tick();
  
  for (int i = 0; i < NUMPIXELS; i++)
  {
    //int ii = (i + ledIndex) % NUMPIXELS;
    int ii = i - ledIndex;
    if (ii < 0)
    {
      ii += NUMPIXELS;
    }
   
    float c = seqValues[ii % seqLength];
    c *= brightness;

    setPixelColor(i, getColor(c * redValue, c * greenValue, c * blueValue));
  }
}

void pong()
{
  tick();
  
  byte tailLength = 30;
  bool isUp = tickCount % 2 == 0;
  
  for (int i = 0; i < NUMPIXELS; i++)
  {
    int ii = (i + ledIndex) % NUMPIXELS;
    float c;
    
    if (isUp)
    { 
      c = 1 - inverseLerp(ledIndex, ledIndex + tailLength, i); 
    }
    else
    {
      int inv = NUMPIXELS - ledIndex;
      c = 1 - inverseLerp(inv, inv + tailLength, i); 
    }
    
    c *= brightness;

    setPixelColor(i, getColor(c * redValue, c * greenValue, c * blueValue));
  }
}

void solid()
{
  tick();

  float c = brightness;
  uint32_t color = getColor(c * redValue, c * greenValue, c * blueValue);
  setColorToAll(color);
}

void rainbow(int segmentLength)
{
  tick();

  for (int i = 0; i < NUMPIXELS; i++)
  {
    int ii = (i + ledIndex) % NUMPIXELS;
    byte c = 255.0 * i / segmentLength;

    setPixelColor(ii, wheel(c, brightness));
  }
}

void rainbowWaves(int segmentLength)
{
  tick();

  for (int i = 0; i < NUMPIXELS; i++)
  {
    int ii = (i + ledIndex) % NUMPIXELS;
    byte c = 255.0 * i / segmentLength;

    float b = 1.0 * ledIndex / NUMPIXELS;
    b = sin(b * 3.1459);

    setPixelColor(ii, wheel(c, b * brightness));
  }
}

void drops(int segmentLength)
{
  tick();
  for (int i = 0; i < NUMPIXELS; i++)
  {
    int ii = (i + ledIndex) % NUMPIXELS;
    int segmentCount = NUMPIXELS / segmentLength;
    int o = ii / segmentCount;
    float c = 1.0 * (i % segmentLength) / segmentLength;
    c *= brightness;

    setPixelColor(ii, getColor(c * redValue, c * greenValue, c * blueValue));
  }
}

void centerWave(int segmentLength)
{
  tick();

  int segmentCount = NUMPIXELS / segmentLength;
  for (int i = 0; i < CENTERPIX; i++)
  {
    int ii = (i + ledIndex) % CENTERPIX;
    float c = sin(PI * ii * segmentLength / CENTERPIX);
    c = (c + 1) * 0.5; 
    c *= sin(PI * i / CENTERPIX);
    c *= brightness;

    setPixelColor(i, getColor(c * redValue, c * greenValue, c * blueValue));
  }

  copyHalfStrip();
}

void randomSparks(int segmentLength)
{
  if (isTickEnd)
  {
    isTickEnd = false;
    for (int i = 0; i < NUMPIXELS; i++)
    {
      stripValues[i] = random(255);
    }
  }

  tick();

  const byte speed = 30;

  for (int i = 0; i < NUMPIXELS; i++)
  {
    byte value = stripValues[i];
    value -= speed;
    stripValues[i] = value;
  
    float c = 1.0 * value / 255;
    c *= brightness;

    setPixelColor(i, c);
  }
}


void copyHalfStrip()
{
  for (int i = CENTERPIX; i < NUMPIXELS; i++)
  {
    setPixelColor(i, getPixelColor(NUMPIXELS - i));
  }
}

void stroboFade(int fadeDurationTicks)
{
  tick(fadeDurationTicks);

  float c = 1 - 1.0 * ledIndex / fadeDurationTicks;
  c = 0.2 + c * 0.8;
  c *= brightness;

  setColorToAll(getColor(c * redValue, c * greenValue, c * blueValue));
}

void strobo(int fadeDurationTicks)
{
  tick(fadeDurationTicks);

  float c = ledIndex * 2 / fadeDurationTicks;
  c *= brightness;

  setColorToAll(getColor(c * redValue, c * greenValue, c * blueValue));
}

void pulse(int fadeDurationTicks)
{
  tick(fadeDurationTicks);

  float c = 1.0 * ledIndex / fadeDurationTicks;
  c = sin(c * 3.1459);
  c = 0.2 + c * 0.8;
  c *= brightness;

  setColorToAll(getColor(c * redValue, c * greenValue, c * blueValue));
}

void pingPong(int segmentLength)
{
  tick();

  for (int i = 0; i < NUMPIXELS; i++)
  {
    int ii = (i + ledIndex) % NUMPIXELS;
    int segmentCount = NUMPIXELS / segmentLength;
    int o = ii / segmentCount;
    //float c = 1.0 * (i % segmentLength) / segmentLength;
    float c = i > ledIndex ? 1.0 : 0.0;
    c *= brightness;

    setPixelColor(i, getColor(c * redValue, c * greenValue, c * blueValue));
  }
}

// ==========================================================

void setColorToAll(uint32_t color)
{
  for (int i = 0; i < NUMPIXELS; i++)
  {
    setPixelColor(i, color);
  }
}

void tick()
{
  tick(NUMPIXELS);  
}

void tick(int cap)
{
  float a = getAcceleration() * float(ledIndex) / cap;
  incrementLedIndex(getIncrementStep() + a);
  //ledIndex += (int)incrementStep;
  if (ledIndex >= cap)
  {
    tickEnd();
  }
}

void incrementLedIndex(float speed)
{
  ledIndexFloat += speed;
  ledIndex = round(ledIndexFloat);
}

float getAcceleration()
{
  if (isOverdrive)
  {
    return overdriveAcceleration;
  }
  return acceleration;
}

int getIncrementStep()
{
  return isOverdrive ? 3 : 1;
}

void tickEnd()
{
  log("tick end");
  updateColorSeqEnd(colorMode);
  tickCount++;
  ledIndex = 0;
  ledIndexFloat = 0;
  isTickEnd = true;
  sequenceStep = 0;

  if (isOverdrive)
  {
    remainingOverdriveLoopCount -= 1;
    if (remainingOverdriveLoopCount < 0)
    {
      log("overdrive end");
      isOverdrive = false;
      choseRandomSequence();
    }

    return;
  }

  if (mode == 255
      && tickCount % switchAutoModeEveryTickCount == 0)
  {
    choseRandomSequence();
  }
}

void choseRandomSequence()
{
  if (seqCount <= 1) return;

  while (true)
  {
    int newMode = random(seqCount);
    if (newMode != randomMode)
    {
      randomMode = newMode;
      break;
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t wheel(byte WheelPos, float brightness) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return getColor((255 - WheelPos * 3) * brightness, 0, WheelPos * 3 * brightness);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return getColor(0, (WheelPos * 3) * brightness, (255 - WheelPos * 3) * brightness);
  }
  WheelPos -= 170;
  return getColor(WheelPos * 3 * brightness, (255 - WheelPos * 3) * brightness, 0);
}

void rgbFromWheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    redValue = 255 - WheelPos * 3;
    greenValue = 0;
    blueValue = WheelPos * 3;
    return;
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    redValue = 0;
    greenValue = WheelPos * 3;
    blueValue = 255 - WheelPos * 3;
    return;
  }
  WheelPos -= 170;
  redValue = WheelPos * 3;
  greenValue = 255 - WheelPos;
  blueValue = 0;
}

void readDMX()
{
#if defined(DMX_ON)

  if (!DMXSerial.receive()) return;
  
  byte brightnessValue = DMXSerial.read(0 + DMXSTART);
  brightness = brightnessValue / 255.0;
  
  redValueDMX = DMXSerial.read(1 + DMXSTART);
  greenValueDMX = DMXSerial.read(2 + DMXSTART);
  blueValueDMX = DMXSerial.read(3 + DMXSTART);
  speed = DMXSerial.read(4 + DMXSTART);
  mode = DMXSerial.read(5 + DMXSTART);
  stroboMode = DMXSerial.read(6 + DMXSTART);
  /*
  if (brightness > 128)
  {
    digitalWrite(LED_BUILTIN, HIGH);
  }
  else
  {
    digitalWrite(LED_BUILTIN, LOW);
  }*/

#endif
}

bool tickStepTime(float stepDuration)
{
  unsigned long ms = getMs();
  unsigned long elapsedMs = ms - tickStartMs;
  normalizedTickTime = elapsedMs / (stepDuration * 1000);
  ledIndex = (int)(normalizedTickTime * NUMPIXELS) % NUMPIXELS;
  if (normalizedTickTime >= 1)
  {
    tickStepTimeEnd();
    return true;
  }

  return false;
}

void tickStepTimeEnd()
{
  String msg = "tick step time end. ms=";
  msg = msg + getMs();
  log(msg);
  ledIndex = 0;
  normalizedTickTime = 0;
  sequenceStep += 1;
  tickStartMs = getMs();
}

unsigned long getMs()
{
  #if defined(LED_NEOPIXEL)
  return (unsigned long)(millis() * 1.25);
  #else
  return millis();
  #endif
}

void testAscii()
{
  int steps = 10;
  for (int i = 0; i < steps; i++)
  {
    byte brightness = (byte)(i * 255 / (steps - 1)); 
    Serial.print(brightnessToAscii(brightness));
  }
  
  Serial.println("end test");
}

void testSequence()
{
  for (int i = 0; i < NUMPIXELS; i++)
  {
    setPixelColor(i, getColor(128, 0, 0));
  }
}

void logStrip()
{
#if defined(LED_SIM_ONLY) 

  int width = 100;
  float step = 1.0 * NUMPIXELS / width;
  int ledIndex = 0;
  float ledIndexFloat = 0;

  Serial.print("[");

  for (int i = 0; i < width; i++)
  {
    ledIndexFloat += step;
    int prevIndex = ledIndex;
    ledIndex = (int)round(ledIndexFloat);
    int brightness = 0;

    for (int j = prevIndex; j < ledIndex; j++)
    {
      uint32_t color = strip[j];
      int v = (color >> 16) & 0xff; // red
      v = max(v, (int)((color >> 8) & 0xff)); // green
      v = max(v, (int)(color & 0xff));
      
      /*int r = (color >> 16) & 0xff; // red
      int g = (int)((color >> 8) & 0xff); // green
      int b = (int)(color & 0xff);
      int v = r + g + b;
      v /= 3;*/

      brightness += v;
    }

    brightness /= ledIndex - prevIndex;

    Serial.print(brightnessToAscii((byte)brightness));
    //Serial.print((byte)brightness);
    //Serial.print("-");
  }

  Serial.println("]");

#endif
}

char* brightnessToAscii(byte brightness)
{
  const int stepCount = 5;
  // byte value = (byte)round(1.0 * brightness * (stepCount - 1) / 255);
  byte value = brightness * (stepCount - 1) / 255;
  switch (value)
  {
    case 0: return "_";
    case 1: return "\u2591";
    case 2: return "\u2592";
    case 3: return "\u2593";
    case 4: return "\u2588";
    default: return "?";
  }
}

void log(char *message)
{
#if !defined(DMX_ON)
  Serial.println(message);
#endif
}

void log(String &message)
{
#if !defined(DMX_ON)
  Serial.println(message);
#endif
}

uint32_t getColor(float r, float g, float b)
{
  return ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
}

void setPixelColor(int i, float c)
{
  c *= brightness;
  setPixelColor(i, getColor(c * redValue, c * greenValue, c * blueValue));
}

void setPixelColor(int i, uint32_t color)
{
#if defined(LED_SIM_ONLY)
  strip[i] = color;
#else

#if defined(LED_NEOPIXEL)
  strip.setPixelColor(i, color);
#else
  strip.setPixel(i, color, 0);
#endif

#endif
}

uint32_t getPixelColor(int i)
{
#if defined(LED_SIM_ONLY)
  return strip[i];
#else

#if defined(LED_NEOPIXEL)
  return strip.getPixelColor(i);
#else
  return strip.getPixel(i);
#endif

#endif
}