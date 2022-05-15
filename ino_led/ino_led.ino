#include <DMXSerial.h>
#include <DMXSerial_avr.h>
#include <DMXSerial_megaavr.h>
#include <Adafruit_NeoPixel.h>

#define PIN        4
#define NUMPIXELS 900
#define DMXSTART 139
#define DMXLENGTH (512)

Adafruit_NeoPixel strip(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

byte redValue = 128;
byte greenValue = 0;
byte blueValue = 255;
float brightness = 0.1;
byte stroboMode = 0;
byte mode = 0;
int delayTicks = 1;
byte speed = 127;

bool tick;
int index;

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

  updateMode(mode);
  //rainbow(50);

  strip.show();
  delay(delayTicks);
}

void readDMX()
{
  byte brightnessValue = DMXSerial.read(0 + DMXSTART);
  brightness = brightnessValue / 255.0;
  
  redValue = DMXSerial.read(1 + DMXSTART);
  greenValue = DMXSerial.read(2 + DMXSTART);
  blueValue = DMXSerial.read(3 + DMXSTART);
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

void updateMode(byte mode)
{
  if (mode > 200)
  {
    pulse(20);
  }
  else if (mode > 170)
  {
    stroboFade(10);
  }
  else if (mode > 170)
  {
    stroboFade(10);
  }
  else if (mode > 140)
  {
    drops(20);
  }
  else if (mode > 100)
  {
    pingPong(20);
  }
  else if (mode >= 0)
  {
    rainbow(50);
  }
}

void rainbow(int segmentLength)
{
  incrementIndex(NUMPIXELS);

  for (int i = 0; i < NUMPIXELS; i++)
  {
    int ii = (i + index) % NUMPIXELS;
    byte c = 255.0 * i / segmentLength;

    strip.setPixelColor(ii, wheel(c, brightness));
  }
}

void rainbowWaves(int segmentLength)
{
  incrementIndex(NUMPIXELS);

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
  incrementIndex(NUMPIXELS);

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
  incrementIndex(fadeDurationTicks);

  float c = 1 - 1.0 * index / fadeDurationTicks;
  c = 0.2 + c * 0.8;
  c *= brightness;

  setColorToAll(strip.Color(c * redValue, c * greenValue, c * blueValue));
}

void strobo(int fadeDurationTicks)
{
  incrementIndex(fadeDurationTicks);

  float c = index * 2 / fadeDurationTicks;
  c *= brightness;

  setColorToAll(strip.Color(c * redValue, c * greenValue, c * blueValue));
}

void pulse(int fadeDurationTicks)
{
  incrementIndex(fadeDurationTicks);

  float c = 1.0 * index / fadeDurationTicks;
  c = sin(c * 3.1459);
  c = 0.2 + c * 0.8;
  c *= brightness;

  setColorToAll(strip.Color(c * redValue, c * greenValue, c * blueValue));
}

void pingPong(int segmentLength)
{
  incrementIndex(NUMPIXELS);

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

void incrementIndex(int cap)
{
  index++;
  if (index >= cap)
  {
    index = 0;
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
