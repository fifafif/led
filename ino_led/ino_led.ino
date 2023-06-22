//#define DMX_ON
//#define LED_SIM_ONLY

#if defined(ARDUINO_AVR_MEGA2560) || defined(ARDUINO_AVR_MEGA) || defined(ARDUINO_AVR_NANO) || defined(ARDUINO_AVR_UNO)
#define LED_NEOPIXEL
#define ARDUINO_LOGIC
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

#include "colors.h"
#include "sequences.h"
#include "serial_com.h"
#include "debug.h"

#if defined(ARDUINO_LOGIC)
#define LED_ONBOARD LED_BUILTIN
#define BUTTON_IN 3
#else
#define LED_ONBOARD 2
#define BUTTON_IN 19
#endif

#define LED_PIN 4
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

// ============================================= CONFIG ==========================================

const bool IS_SLAVE = true;
const bool IS_OVERDRIVE_REACTIVE = true;
const bool IS_BEAT_REACTIVE = false;

byte redValue = 255;
byte greenValue = 255;
byte blueValue = 0;

// DMX Values
byte redValueDMX = 255;
byte greenValueDMX = 255;
byte blueValueDMX = 0;
float brightness = 0.5;
byte stroboMode = 0;
byte mode = 255;

int delayTicks = 1;

byte seqCount = 17;
byte overdriveSeqCount = 1;
byte randomMode = 0;

// Time
byte speed = 127;
int seqLength = 60;
float acceleration = 1;
byte incrementStep = 1;
int tickCount = 0;
int ledIndex;
float ledIndexFloat = 0;
byte switchAutoModeEveryTickCount = 8;
bool isTickEnd = true;
unsigned long stepStartMs;
float normalizedStepTime;
int ticksSinceLastBeat = 666;
int lastBeatState;
int stepTicks;

// Color
// DMX = 0, Rnd = 1, Wheel = 2, WheelStep = 3, Serial = 4
byte colorMode = 1;
byte colorWheelStep = 255 / 6;
byte colorStepCount = 6;
int colorIndex = 0;
byte colorWheelPosition;

// Sequences
byte stripValues[NUMPIXELS] = {};
int fill = 0;
int sequenceStep;
bool isBeatChangingColor;
bool isBeatResettingSequence;

// Overdrive
bool isOverdrive;
int lastOverdriveButtonState;
int remainingOverdriveLoopCount;
float overdriveAcceleration = 9;
const int overdriveLoopCount = 3;

// ============================================= MAIN ==========================================

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
  Serial.println("LED Start!");
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

  setupSerial(IS_SLAVE);

  if (IS_SLAVE)
  {
    colorMode = 4;
  }
  else
  {
    colorMode = 1;
  }
}

void loop() 
{
  readDMX();
  readBeat();
  readOverrideButton();
  readSerial();

  // Manual color mode override.  
  //colorMode = 1;
  updateColorTick(colorMode);
  
  if (isOverdrive)
  {
    updateSeqOverdrive();
  }
  else
  {
    updateSeq();
  }
  
#if defined(LED_SIM_ONLY)
  logStrip(strip, NUMPIXELS);
  delay(10);
#else
  strip.show();
#endif

  //delay(delayTicks);
}

void readSerial()
{
  if (readSerialMessage())
  {
    switch (getSerialMessageType())
    {
      case 0:
        break;

      case 1:
        sequenceEnd(getSerialMessageSequence());
        break;

      case 2:
        rgbFromWheel(getSerialMessageColor());
        break;

      case 3:
        startOverdrive();
        randomMode = getSerialMessageSequence();
        break;

    }
    log("Message!");
  }
}

void readBeat()
{
  if (!IS_BEAT_REACTIVE
      || IS_SLAVE) return;

  ticksSinceLastBeat += 1;
  int beatState = digitalRead(BEAT_IN);
  digitalWrite(LED_ONBOARD, beatState);
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

  if (isOverdrive) return;
  if (getMs() - stepStartMs < 200) return;

  ticksSinceLastBeat = 0;

  if (isBeatResettingSequence)
  {
    stepTimeEnd();
  }
  else if (isBeatChangingColor)
  {
    changeColor();
    writeSerialColor();
  }
  else
  {
    sequenceEnd();
  }
}

void readOverrideButton()
{
  if (!IS_OVERDRIVE_REACTIVE
      || IS_SLAVE) return;

  int buttonState = digitalRead(BUTTON_IN);
  if (buttonState == HIGH
      && buttonState != lastOverdriveButtonState)
  {
    startOverdrive();  
  }

  lastOverdriveButtonState = buttonState;
}

void startOverdrive()
{
  log("start overdrive!");
  sequenceEnd();
  startStepTime();
  isOverdrive = true;
  remainingOverdriveLoopCount = overdriveLoopCount;
  choseRandomOverdrive();  

  if (!IS_SLAVE)
  {
    writeSerialMessageOverdrive(randomMode, colorWheelPosition);
  }
}

void updateSeqOverdrive()
{
  int s = randomMode;
  
  switch(s)
  {
    case 0: fireworks(); break;
  }
}

void updateSeq()
{
  int s = int(seqCount) * 255 / mode;
  if (s == seqCount)
  {
    s = randomMode;
  }
  //s = 3;
  
  switch(s)
  {
    case 0: solid(); break;
    case 1: pulse(40); break;
    case 2: stroboFade(10); break;
    case 3: grow(0.5f); break;
    case 4: grow(2.0f); break;
    case 5: pingPong(20); break; // fix
    case 6: wavesSeq(); break;
    case 7: wavesHalfSeq(); break; // fix
    case 8: wavesSqrtSeq(); break;
    case 9: wavesHardSeq(); break;
    case 10: rainbow(60); break;  
    case 11: rainbow(900); break;
    case 12: dropsTime(30); break;
    case 13: firebolt(100); break;
    case 14: cylon(100); break;
    case 15: centerWave(60); break;
    case 16: randomSparks(60); break; 
  }
}

// ===============================================================================================
// ============================================= SEQUENCES =======================================
// ===============================================================================================

void dropsTime(int segmentLength)
{
  updateStepTime(4.0f, true);

  for (int i = 0; i < NUMPIXELS; i++)
  {
    int ii = (i + ledIndex) % NUMPIXELS;
    int segmentCount = NUMPIXELS / segmentLength;
    float c = 1.0 * (i % segmentLength) / segmentLength;
    setPixelColor(ii, c);
  }
}

void cylon(int length)
{
  if (updateStepTime(2.0f, true)) return;

  bool isReverse = normalizedStepTime > 0.5;
  if (isReverse)
  {
    normalizedStepTime -= 0.5;
  }

  normalizedStepTime *= 2;

  ledIndex = easeOut(normalizedStepTime) * NUMPIXELS;
  length = lerp(length / 2, length, normalizedStepTime);

  for (int i = 0; i < NUMPIXELS; i++)
  {
    float c = inverseLerp(ledIndex - length, ledIndex, i);
    c = easeIn(c);
    if (i > ledIndex)
    {
      c = 0;
    }

    if (isReverse) {
      setPixelColor(i, c);
    } else {
      setPixelColor(NUMPIXELS - i - 1, c);
    }
  }
}

void firebolt(int length)
{
  if (isTickEnd)
  {
    generateRandomStripValues();
  }

  if (updateStepTime(2.0f, true)) return;

  ledIndex = easeOut(normalizedStepTime) * NUMPIXELS;

  for (int i = 0; i < NUMPIXELS; i++)
  {
    float c = inverseLerp(ledIndex - length, ledIndex, i);
    c = easeOut(c);
    if (i > ledIndex)
    {
      c = 0;
    }
    else
    {
      byte value = stripValues[i];
      value -= 10;
      stripValues[i] = value;
      c *= value / 255.0;
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
  if (updateStepTime(1.0f))
  {
    generateRandomStripValues();
    return;
  }
  
  ledIndex = easeOutSine(normalizedStepTime) * NUMPIXELS;

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
  if (updateStepTime(0.15)) return;
  
  ledIndex = NUMPIXELS - easeOut(normalizedStepTime) * NUMPIXELS;

  for (int i = 0; i < NUMPIXELS; i++)
  {
    float c = inverseLerp(ledIndex - 30, ledIndex, i); 
    setPixelColor(i, c);
  }
}

void fireworksStep3()
{
  if (updateStepTime(2, true)) return;

  float exploTime = normalizedStepTime * 4;
  float exploFactor = easeOut(1 - clamp01(exploTime));

  for (int i = 0; i < NUMPIXELS; i++)
  {
    byte value = stripValues[i];
    value -= 10;
    stripValues[i] = value;
  
    float c = 1.0 * value / 255;
    c *= easeIn(1 - normalizedStepTime);

    c = lerp(c, 1.0, exploFactor);

    setPixelColor(i, c);
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
  updateStepTime(6.0f, true);

  ledIndex = easeInSine(normalizedStepTime) * NUMPIXELS;
  
  for (int i = 0; i < NUMPIXELS; i++)
  {
    int ii = i - ledIndex;
    if (ii < 0)
    {
      ii += NUMPIXELS;
    }
   
    float c = seqValues[ii % seqLength];

    setPixelColor(i, c);
  }
}

void grow(float duration)
{
  isBeatChangingColor = true;
  isBeatResettingSequence = true;

  if (updateStepTime(duration, 2, false)) return;
  
  byte tailLength = 30;
  bool isUp = tickCount % 2 == 0;
  if (isUp)
  {
    ledIndex = easeIn(normalizedStepTime) * NUMPIXELS;
  }
  else
  {
    ledIndex = easeOut(normalizedStepTime) * NUMPIXELS;
  }
  
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
    
    setPixelColor(i, c);
  }
}

void solid()
{
  updateStepTime(1.0f, true);

  float c = brightness;
  uint32_t color = getColor(c * redValue, c * greenValue, c * blueValue);
  setColorToAll(color);
}

void rainbow(int segmentLength)
{
  updateStepTime(4.0f, true);

  for (int i = 0; i < NUMPIXELS; i++)
  {
    int ii = (i + stepTicks) % NUMPIXELS;
    byte c = 255.0 * i / segmentLength;

    setPixelColor(ii, wheel(c, brightness));
  }
}

void rainbowWaves(int segmentLength)
{
  updateStepTime(4.0f, true);

  for (int i = 0; i < NUMPIXELS; i++)
  {
    int ii = (i + ledIndex) % NUMPIXELS;
    byte c = 255.0 * i / segmentLength;

    float b = 1.0 * ledIndex / NUMPIXELS;
    b = sin(b * 3.1459);

    setPixelColor(ii, wheel(c, b * brightness));
  }
}

void centerWave(int segmentLength)
{
  if (updateStepTime(3.0f, true)) return;

  for (int i = 0; i <= CENTERPIX; i++)
  {
    int ii = (i + ledIndex) % CENTERPIX;
    float c = sin(PI * ii * segmentLength / CENTERPIX);
    c = (c + 1) * 0.5; 
    c *= sin(PI * i / CENTERPIX);

    setPixelColor(i, c);
  }

  copyHalfStrip();
}

void randomSparks(int segmentLength)
{
  if (isTickEnd)
  {
    generateRandomStripValues();
  }

  updateStepTime(1.0f, true);

  const byte speed = 3;

  for (int i = 0; i < NUMPIXELS; i++)
  {
    byte value = stripValues[i];
    value -= speed;
    stripValues[i] = value;
  
    float c = (1 - normalizedStepTime) * value / 255;

    setPixelColor(i, c);
  }
}

void stroboFade(int fadeDurationTicks)
{
  updateStepTime(1.0f, true);

  float c = 1 - normalizedStepTime;
  c = 0.2 + c * 0.8;
  setColorToAll(c);
}

void strobo(int fadeDurationTicks)
{
  updateStepTime(1.0f, true);

  float c = ledIndex * 2 / fadeDurationTicks;
  setColorToAll(c);
}

void pulse(int fadeDurationTicks)
{
  updateStepTime(1.0f, true);

  float c = normalizedStepTime;
  c = sin(c * PI);
  c = 0.2 + c * 0.8;
  setColorToAll(c);
}

void pingPong(int segmentLength)
{
  updateStepTime(1.0f, 2);

  for (int i = 0; i < NUMPIXELS; i++)
  {
    int ii = (i + ledIndex) % NUMPIXELS;
    int segmentCount = NUMPIXELS / segmentLength;
    int o = ii / segmentCount;
    //float c = 1.0 * (i % segmentLength) / segmentLength;
    float c = i > ledIndex ? 1.0 : 0.0;

    setPixelColor(i, c);
  }
}

// ===============================================================================================
// ============================================= TIME ============================================
// ===============================================================================================

void sequenceEnd()
{
  /*if (IS_SLAVE)
  {
    return;
  }*/

  log("sequence end");
  changeColor();
  ledIndex = 0;
  ledIndexFloat = 0;
  isTickEnd = true;
  sequenceStep = 0;

  if (isOverdrive)
  {
    log("overdrive end");
    isOverdrive = false;
    choseRandomSequence();
    return;
  }

  if (mode == 255
      && tickCount % switchAutoModeEveryTickCount == 0)
  {
    choseRandomSequence();
  }

  if (!IS_SLAVE)
  {
    writeSerialMessage(1, randomMode, colorWheelPosition);
  }
}

void sequenceEnd(byte sequenceMode)
{
  sequenceEnd();
  randomMode = sequenceMode;
}

bool updateStepTime(float stepDuration, int stepCount, bool isChangingColor)
{
  if (updateStepTime(stepDuration))
  {
    if (stepCount <= sequenceStep)
    {
      sequenceEnd();
    }
    else if (isChangingColor)
    {
      if (!IS_SLAVE)
      {
        changeColor();
        writeSerialColor();
      }
    }
    return true;
  }

  return false;
}

bool updateStepTime(float stepDuration)
{
  return updateStepTime(stepDuration, false);
}

bool updateStepTime(float stepDuration, bool isLastStep)
{
  unsigned long ms = getMs();
  unsigned long elapsedMs = ms - stepStartMs;
  normalizedStepTime = elapsedMs / (stepDuration * 1000);
  ledIndex = (int)(normalizedStepTime * NUMPIXELS) % NUMPIXELS;
  isTickEnd = false;
  stepTicks += 1;

  if (normalizedStepTime >= 1)
  {
    stepTimeEnd();
    if (isLastStep)
    {
      sequenceEnd();
    }
    return true;
  }

  return false;
}

void stepTimeEnd()
{
  String msg = "step end. ms=";
  msg = msg + getMs();
  log(msg);
  startStepTime();
  sequenceStep += 1;
  tickCount += 1;
}

void startStepTime()
{
  ledIndex = 0;
  stepTicks = 0;
  normalizedStepTime = 0;
  stepStartMs = getMs();
}

void choseRandomOverdrive()
{
  randomMode = random(overdriveSeqCount);
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

  String msg = "new sequence: ";
  msg += randomMode;
  log(msg);
}

unsigned long getMs()
{
  #if defined(LED_NEOPIXEL)
  return (unsigned long)(millis() * 1.25);
  #else
  return millis();
  #endif
}

void testSequence()
{
  for (int i = 0; i < NUMPIXELS; i++)
  {
    setPixelColor(i, getColor(128, 0, 0));
  }
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

void copyHalfStrip()
{
  for (int i = CENTERPIX; i < NUMPIXELS; i++)
  {
    setPixelColor(i, getPixelColor(NUMPIXELS - i));
  }
}

void rgbFromWheel(byte wheelPos)
{
  colorWheelPosition = wheelPos;
  rgbFromWheel(wheelPos, redValue, greenValue, blueValue);
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
    rgbFromWheel(colorIndex * 255 / colorStepCount);
  }
  else if (colorMode == 1)
  {
    rgbFromWheel(random(255));
  }
  else if (colorMode == 4)
  {
    rgbFromWheel(getSerialMessageColor());
  }
}

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

void writeSerialColor()
{
  if (IS_SLAVE) return;

  writeSerialMessageColor(colorWheelPosition);
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


void generateRandomStripValues()
{
  for (int i = 0; i < NUMPIXELS; i++)
  {
    stripValues[i] = random(255);
  }
}

void setColorToAll(uint32_t color)
{
  for (int i = 0; i < NUMPIXELS; i++)
  {
    setPixelColor(i, color);
  }
}

void setColorToAll(float c)
{
  c *= brightness;
  uint32_t color = getColor(c * redValue, c * greenValue, c * blueValue);
  for (int i = 0; i < NUMPIXELS; i++)
  {
    setPixelColor(i, color);
  }
}


