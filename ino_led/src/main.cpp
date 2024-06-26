//#define DMX_ON
// #define IS_WIFI_ENABLED

#define PROD

// LED Debug
#define LED_SIM_ONLY
#define LED_SIM_DEBUG
#define LED_SIM_PRINT
// #define LED_SIM_PRINT_BYTES
#define LED_SIM_PRINT_BYTES_BRIGHTNESS
// #define BEAT_SIMULATOR

#if defined(PROD)
#undef LED_SIM_ONLY
#undef BEAT_SIMULATOR
#endif

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
#include "beat_simulator.h"
#include "overdrive_handler.h"

#if defined(IS_WIFI_ENABLED)
#include "wifi_web_server.h"
#endif

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
const bool IS_OVERDRIVE_REACTIVE = true;
const bool IS_BEAT_REACTIVE = false;

// DMX Values
byte redValueDMX = 255;
byte greenValueDMX = 255;
byte blueValueDMX = 0;

#if defined(PROD)
float brightness = 0.06f;
#else
float brightness = 1;
#endif

byte stroboMode = 0;
byte mode = 255;

int delayTicks = 1;

byte randomMode = 0;

// Time
byte speed = 127;
int ticksSinceLastBeat = 666;
int lastBeatState;

// Sequences
byte stripValues[NUMPIXELS] = {};

StripHandler stripHandler;
Playback playback(NUMPIXELS);
Animations animations(&playback, &stripHandler);
OverdriveHandler overdriveHandler(&animations);

#if defined(BEAT_SIMULATOR)
BeatSimulator beatSimulator(&animations, 145);
#endif

#if defined(IS_WIFI_ENABLED)
//WifiServer wifiServer;
// Animations *WifiServer::animations = animations;  
// StripHandler *WifiServer::strip = strip;  
// WifiServer wifiServer(&animations, &stripHandler);  
#endif

// Overdrive
bool isOverdrive;
int lastOverdriveButtonState;

void readSerial();
void readBeat();
void beat();
void readOverrideButton();
// void startOverdrive();
void startOverdrive(byte index);
void writeSerialColor();
void readDMX();
void showStrip();

// ============================================= MAIN ==========================================

void setup () 
{
  pinMode(LED_ONBOARD, OUTPUT);
  // pinMode(BUTTON_IN, INPUT);
  pinMode(BEAT_IN, INPUT);
  randomSeed(analogRead(0) + analogRead(1));
  
  stripHandler.pixelCount = NUMPIXELS;

#if defined(LED_SIM_ONLY)
  stripHandler.strip = strip;
#else
  stripHandler.strip = &strip;
#endif
  
  stripHandler.brightness = brightness;
  stripHandler.stripValues = stripValues;

  overdriveHandler.setup();


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

#if defined(IS_WIFI_ENABLED)
  setupWifi(&stripHandler, &playback);
#endif

  setupSerial(IS_SLAVE);
  animations.setAsSlave(IS_SLAVE);

  overdriveHandler.isSlave = IS_SLAVE;
  overdriveHandler.isOverdriveReactive = IS_OVERDRIVE_REACTIVE;
}

void loop() 
{
  readDMX();
  readBeat();
  // readOverrideButton();
  readSerial();

  overdriveHandler.update();

#if defined(BEAT_SIMULATOR)
  beatSimulator.update();
#endif

  animations.update();

#if defined(IS_WIFI_ENABLED)
  loopWifi();
#endif

  showStrip();
  delay(1);
}

uint32_t *pix = new uint32_t[NUMPIXELS];

void showStrip()
{
#if defined(LED_SIM_ONLY)
  #if defined(LED_SIM_PRINT)
    #if defined(LED_SIM_PRINT_BYTES)
  logStripBytes(strip, NUMPIXELS);
  delay(10);
    #else
  logStrip(strip, NUMPIXELS);
  delay(10);
    #endif
  #endif
#else

  // for (int i = 0; i < NUMPIXELS; i++)
  // {
  //   pix[i] = strip.getPixelC(i);
  // } 

  // logStrip(pix, NUMPIXELS);
  

  strip.show();
#endif
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
        animations.startNewAnimation(getSerialMessageSequence());
        break;

      case 2:
        animations.updateSlaveColor(getSerialMessageColor());
        break;

      case 3:
        startOverdrive(getSerialMessageSequence());
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
  if (getMs() - playback.stepStartMs < 200) return;

  ticksSinceLastBeat = 0;
}

void startOverdrive()
{
  log("start overdrive!");

  //animations.startOverdriveRandom(0);

  if (!IS_SLAVE)
  {
    writeSerialMessageOverdrive(animations.currentOverdriveAnimationIndex, stripHandler.colorWheelPosition);
  }
}

void startOverdrive(byte index)
{
  animations.currentOverdriveAnimationIndex = index;
  startOverdrive();
}

void writeSerialColor()
{
  if (IS_SLAVE) return;

  writeSerialMessageColor(stripHandler.colorWheelPosition);
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



