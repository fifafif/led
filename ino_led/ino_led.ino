//#define DMX_ON
#define LED_SIM_ONLY
#define LED_SIM_PRINT_BYTES

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

#include "playback.h"
#include "colors.h"
#include "sequences.h"
#include "serial_com.h"
#include "debug.h"
#include "tick_payload.h"
#include "strip_handler.h"
#include "animations.h"

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

const bool IS_SLAVE = false;
const bool IS_OVERDRIVE_REACTIVE = false;
const bool IS_BEAT_REACTIVE = false;

byte redValue = 255;
byte greenValue = 255;
byte blueValue = 0;

// DMX Values
byte redValueDMX = 255;
byte greenValueDMX = 255;
byte blueValueDMX = 0;
float brightness = 1;
byte stroboMode = 0;
byte mode = 255;

int delayTicks = 1;

const byte seqCount = 19;
const byte overdriveSeqCount = 3;
const byte slaveColorModeCount = 4;
byte randomMode = 0;

// Time
byte speed = 127;
int seqLength = 60;
float acceleration = 1;
byte incrementStep = 1;
int tickCount = 0;
int ledIndex;
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

StripHandler handler;
Playback playback(NUMPIXELS);
Animations animations(&playback, &handler);

// Overdrive
bool isOverdrive;
int lastOverdriveButtonState;
int remainingOverdriveLoopCount;
float overdriveAcceleration = 9;
const int overdriveLoopCount = 3;

// ============================================= MAIN ==========================================

void setup () 
{
  handler.numPixels = NUMPIXELS;
  handler.strip = strip;
  handler.brightness = brightness;
  handler.stripValues = stripValues;

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

  // testSequence();

#if defined(LED_SIM_ONLY)
#if defined(LED_SIM_PRINT_BYTES)
  logStripBytes(strip, NUMPIXELS);
  delay(10);
#else
  logStrip(strip, NUMPIXELS);
  delay(10);
#endif
#else
  strip.show();
#endif
}

byte slaveColorMode = 0;

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
        updateSlaveColor(getSerialMessageColor());
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
  /*
  switch(s)
  {
    case 0: fireworks(); break;
    case 1: chargeSequence(5, 20); break;
    case 2: randomSparksOverdrive(); break;
  }*/
}

void updateSeq()
{
  int s = int(seqCount) * 255 / mode;
  if (s == seqCount)
  {
    s = randomMode;
  }
  //s = 3;
  
  animations.update();
}

// ===============================================================================================
// ============================================= TIME ============================================
// ===============================================================================================

void sequenceEnd()
{
  log("sequence end");
  
  if (IS_SLAVE)
  {
    slaveColorMode = random(slaveColorModeCount);
  }
  
  changeColor();
  ledIndex = 0;
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
    setPixelColor(i, getColor(256.0f * i / NUMPIXELS, 0, 0));
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
  return strip.getPixelC(i);
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
    updateSlaveColor(getSerialMessageColor());
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

  rgbFromWheel(slaveColor);        
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

#endif
}

int reverseIndex(int i)
{
  return NUMPIXELS - i - 1;
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


