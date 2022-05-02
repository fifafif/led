#include <Adafruit_NeoPixel.h>
#define PIN        4
#define NUMPIXELS 300

Adafruit_NeoPixel strip(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

byte redValue = 128;
byte greenValue = 0;
byte blueValue = 255;
float brightness = 0.1;
int delayTicks = 10;

int index;

void setup() 
{
  strip.begin();
}

void loop() 
{  
  //pulse(20);
  //stroboFade(10);
  //strobo(20);
  //drops(20);  
  //pingPong(120);
  //rainbow(300);
  rainbow(50);
  
  strip.show();
  delay(delayTicks);
}

void rainbow(int segmentLength) 
{  
  incrementIndex(NUMPIXELS);

  for(int i=0; i<NUMPIXELS; i++) 
  {
    int ii = (i + index) % NUMPIXELS;
    byte c = 255.0 * i / segmentLength;
    
    strip.setPixelColor(ii, wheel(c, brightness));
  }
}

void rainbowWaves(int segmentLength) 
{  
  incrementIndex(NUMPIXELS);

  for(int i=0; i<NUMPIXELS; i++) 
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

  for(int i=0; i<NUMPIXELS; i++) 
  {
    int ii = (i + index) % NUMPIXELS;
    int segmentCount = NUMPIXELS / segmentLength;
    int o = ii / segmentCount;
    float c = 1.0 * (i % segmentLength) / segmentLength;
    
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

  for(int i=0; i<NUMPIXELS; i++) 
  {
    int ii = (i + index) % NUMPIXELS;
    int segmentCount = NUMPIXELS / segmentLength;
    int o = ii / segmentCount;
    //float c = 1.0 * (i % segmentLength) / segmentLength;
    float c = i > index ? 1.0 : 0.0;
    
    strip.setPixelColor(i, strip.Color(c * redValue, c * greenValue, c * blueValue));
  }
}

// ==========================================================

void setColorToAll(uint32_t color)
{
  for(int i=0; i<NUMPIXELS; i++) 
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
  if(WheelPos < 85) {
    return strip.Color((255 - WheelPos * 3) * brightness, 0, WheelPos * 3 * brightness);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, (WheelPos * 3) * brightness, (255 - WheelPos * 3) * brightness);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3 * brightness, (255 - WheelPos * 3) * brightness, 0);
}
