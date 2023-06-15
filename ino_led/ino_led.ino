#include <DMXSerial.h>
#include <DMXSerial_avr.h>
#include <DMXSerial_megaavr.h>
#include <Adafruit_NeoPixel.h>
#include "sequences.h"
#include "Arduino.h"

#define PIN        4
#define NUMPIXELS 300
#define CENTERPIX NUMPIXELS / 2
#define SEQ_LENGTH 30
#define DMXSTART 139
#define DMXLENGTH (512)

Adafruit_NeoPixel strip(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

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
int index;
float indexFloat = 0;
byte switchAutoModeTick = 8;

// Color
byte colorMode = 1;
byte colorWheelStep = 255 / 6;
byte colorStepCount = 6;
int colorIndex = 0;

// Sequences
int fill = 0;

void setup () 
{
  //Serial.begin(9600);
  //Serial.write("LED Start");
  DMXSerial.init(DMXProbe);
  pinMode(LED_BUILTIN, OUTPUT);
  DMXSerial.maxChannel(DMXLENGTH);
  strip.begin();
}

void loop() 
{
  if (DMXSerial.receive()) 
  {
    readDMX();
  }

  colorMode = 1;

  tick();
  updateColorTick(colorMode);
  updateSeq();
  //updateMode(mode);
 
  strip.show();
  //delay(delayTicks);
}

//void solid();
//void pulse(int x);
//typedef void (*sequenceMap) (void);
//sequenceMap bla[] = {solid, pulse};

void updateSeq()
{
  int s = int(seqCount) * 255 / mode;

  if (s == seqCount)
  {
    s = randomMode;
  }
  //s = 11;
  
  switch(s)
  {
    case 0:
      solid();
      break;

    case 1:
      pulse(40);
      break;

    case 2:
      stroboFade(10);
      break;

    case 3:
      pong();
      break;

    case 4:
      drops(20);
      break;

    case 5:
      pingPong(20);
      break;

    case 6:
      wavesSeq();
      break;

    case 7:
      wavesHalfSeq();
      break;

    case 8:
      wavesSqrtSeq();
      break;

    case 9:
      wavesHardSeq();
      break;

    case 10:
      rainbow(60);
      break;  

    case 11:
      rainbow(20);
      break;
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
    rgbFromWheel(index);
  }
}

void fillUp()
{
  tick();

  int rest = NUMPIXELS - fill;
  for (int i = 0; i < fill; ++i)
  {
    
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
    //int ii = (i + index) % NUMPIXELS;
    int ii = i - index;
    if (ii < 0)
    {
      ii += NUMPIXELS;
    }
   
    float c = seqValues[ii % seqLength];
    c *= brightness;

    strip.setPixelColor(i, strip.Color(c * redValue, c * greenValue, c * blueValue));
  }
}

//void setIntensity

void pong()
{
  tick();
  
  byte tailLength = 30;
  bool isUp = tickCount % 2 == 0;
  
  for (int i = 0; i < NUMPIXELS; i++)
  {
    int ii = (i + index) % NUMPIXELS;
    float c;
    
    if (isUp)
    { 
      c = 1 - inverseLerp(index, index + tailLength, i); 
    }
    else
    {
      int inv = NUMPIXELS - index;
      c = 1 - inverseLerp(inv, inv + tailLength, i); 
    }
    
    c *= brightness;

    strip.setPixelColor(i, strip.Color(c * redValue, c * greenValue, c * blueValue));
  }
}

void solid()
{
  tick();

  float c = brightness;
  uint32_t color = strip.Color(c * redValue, c * greenValue, c * blueValue);
  setColorToAll(color);
}


void rainbow(int segmentLength)
{
  tick();

  for (int i = 0; i < NUMPIXELS; i++)
  {
    int ii = (i + index) % NUMPIXELS;
    byte c = 255.0 * i / segmentLength;

    strip.setPixelColor(ii, wheel(c, brightness));
  }
}

void rainbowWaves(int segmentLength)
{
  tick();

  for (int i = 0; i < NUMPIXELS; i++)
  {
    int ii = (i + index) % NUMPIXELS;
    byte c = 255.0 * i / segmentLength;

    float b = 1.0 * index / NUMPIXELS;
    b = sin(b * 3.1459);

    strip.setPixelColor(ii, wheel(c, b * brightness));
  }
}

void drops(int segmentLength)
{
  tick();
  for (int i = 0; i < NUMPIXELS; i++)
  {
    int ii = (i + index) % NUMPIXELS;
    int segmentCount = NUMPIXELS / segmentLength;
    int o = ii / segmentCount;
    float c = 1.0 * (i % segmentLength) / segmentLength;
    c *= brightness;

    strip.setPixelColor(ii, strip.Color(c * redValue, c * greenValue, c * blueValue));
  }
}

void stroboFade(int fadeDurationTicks)
{
  tick(fadeDurationTicks);

  float c = 1 - 1.0 * index / fadeDurationTicks;
  c = 0.2 + c * 0.8;
  c *= brightness;

  setColorToAll(strip.Color(c * redValue, c * greenValue, c * blueValue));
}

void strobo(int fadeDurationTicks)
{
  tick(fadeDurationTicks);

  float c = index * 2 / fadeDurationTicks;
  c *= brightness;

  setColorToAll(strip.Color(c * redValue, c * greenValue, c * blueValue));
}

void pulse(int fadeDurationTicks)
{
  tick(fadeDurationTicks);

  float c = 1.0 * index / fadeDurationTicks;
  c = sin(c * 3.1459);
  c = 0.2 + c * 0.8;
  c *= brightness;

  setColorToAll(strip.Color(c * redValue, c * greenValue, c * blueValue));
}

void pingPong(int segmentLength)
{
  tick();

  for (int i = 0; i < NUMPIXELS; i++)
  {
    int ii = (i + index) % NUMPIXELS;
    int segmentCount = NUMPIXELS / segmentLength;
    int o = ii / segmentCount;
    //float c = 1.0 * (i % segmentLength) / segmentLength;
    float c = i > index ? 1.0 : 0.0;
    c *= brightness;

    strip.setPixelColor(i, strip.Color(c * redValue, c * greenValue, c * blueValue));
  }
}

// ==========================================================

void setColorToAll(uint32_t color)
{
  for (int i = 0; i < NUMPIXELS; i++)
  {
    strip.setPixelColor(i, color);
  }
}

void tick()
{
  tick(NUMPIXELS);  
}

void tick(int cap)
{
  float a = acceleration * float(index) / cap;
  indexFloat += incrementStep + a;
  index = round(indexFloat);
  //index += (int)incrementStep;
  if (index >= cap)
  {
    tickEnd();
  }
}

void tickEnd()
{
  updateColorSeqEnd(colorMode);
  tickCount++;
  index = 0;
  indexFloat = 0;

  if (mode == 255
      && tickCount % switchAutoModeTick == 0)
  {
    randomMode = random(seqCount);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t wheel(byte WheelPos, float brightness) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return strip.Color((255 - WheelPos * 3) * brightness, 0, WheelPos * 3 * brightness);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, (WheelPos * 3) * brightness, (255 - WheelPos * 3) * brightness);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3 * brightness, (255 - WheelPos * 3) * brightness, 0);
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

float lerp(int a, int b, float t)
{
  if (t >= 1) return b; 
  if (t <= 0) return a;
  
  return a + (b - a) * t;
}

float inverseLerp(int a, int b, float c)
{
  if (b == a) return 0;
  if (c <= a) return 0;
  if (c >= b) return 1;
  
  return (c - a) / (b - a);
}

void readDMX()
{
  byte brightnessValue = DMXSerial.read(0 + DMXSTART);
  brightness = brightnessValue / 255.0;
  
  redValueDMX = DMXSerial.read(1 + DMXSTART);
  greenValueDMX = DMXSerial.read(2 + DMXSTART);
  blueValueDMX = DMXSerial.read(3 + DMXSTART);
  speed = DMXSerial.read(4 + DMXSTART);
  mode = DMXSerial.read(5 + DMXSTART);
  stroboMode = DMXSerial.read(6 + DMXSTART);
  
  if (brightness > 128)
  {
    digitalWrite(LED_BUILTIN, HIGH);
  }
  else
  {
    digitalWrite(LED_BUILTIN, LOW);
  }
}
