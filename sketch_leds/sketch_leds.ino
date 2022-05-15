#include <FastLED.h>

#define LED_PIN     4
#define NUM_LEDS    300

CRGB leds[NUM_LEDS];

byte redValue = 128;
byte greenValue = 0;
byte blueValue = 255;

int index;
bool isOn;

void setup() {

  Serial.begin(9600);
  Serial.write("LED Start");
  pinMode(LED_BUILTIN, OUTPUT);
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
}


void loop() {

  drops(20);
  FastLED.show();
  delay(1);
}

void drops(int segmentLength)
{
  index++;
  if (index >= NUM_LEDS)
  {
    index = 0;
  }

  for(int i=0; i<NUM_LEDS; i++) 
  {
    int ii = (i + index) % NUM_LEDS;
    /*if (ii >= NUMPIXELS)
    {
      ii -= NUMPIXELS;
    }*/

    int segmentCount = NUM_LEDS / segmentLength;

    int o = ii / segmentCount;

    float c = 1.0 * (i % segmentLength) / segmentLength;
    leds[ii] = CRGB(c * redValue, c * greenValue, c * blueValue);
  }
}
