#include "arduinoFFT.h" // Standard Arduino FFT library 

#define AUDIO_IN_PIN 4
#define BEAT_PIN 5
#define LED_PIN 2
int led_pin = 2;//LED_BUILTIN;

#define SCL_INDEX 0x00
#define SCL_TIME 0x01
#define SCL_FREQUENCY 0x02
#define SCL_PLOT 0x03

const uint16_t samples = 512; //This value MUST ALWAYS be a power of 2
const double samplingFrequency = 5000;

const uint16_t amplitude = 1000;
const uint16_t noiseThreshold = 500;
const uint16_t bandMaxValue = 1000;
const uint16_t beatPeakThreshold = 30;

unsigned int sampling_period_us;
unsigned long microseconds;

const uint8_t bandCount = 7;
byte peak[] = {0,0,0,0,0,0,0};
int bandIndices[] = {3, 7, 17, 35, 80, 200, samples >> 2};

double vReal[samples];
double vImag[samples];
unsigned long newTime, oldTime, lastBeatTimeMs;
const int maxBPM = 250;
unsigned int minBeatIntervalMs = 60 * 1000 / maxBPM; 
unsigned int beatSignalDurationMs = 10;
bool isInBeat;

char printBuffer[10];

ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, samples, samplingFrequency);


void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BEAT_PIN, OUTPUT);
  sampling_period_us = round(1000000 * (1.0 / samplingFrequency));
}

void loop2()
{
  double value = analogRead(AUDIO_IN_PIN);  
  Serial.println(value);
  delay(1);
}
void loop() {

  long time = micros();

  for (int i = 0; i < samples; i++) {
    newTime = micros()-oldTime;
    oldTime = newTime;
    unsigned long nextTime = newTime + sampling_period_us;
    vReal[i] = analogRead(AUDIO_IN_PIN); // A conversion takes about 1uS on an ESP32
    vImag[i] = 0;
    while (micros() < (nextTime)) { /* do nothing to wait */ }
  }

  /*Serial.print("Sample");
  Serial.println(micros() - time);
  time = micros();*/

  FFT.windowing(vReal, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.compute(vReal, vImag, samples, FFT_FORWARD);
  FFT.complexToMagnitude(vReal, vImag, samples);

  Serial.print("FFT ");
  Serial.println(micros() - time);
  time = micros();
  int max = 0;
  for (int i = 2; i < (samples/2); i++){ // Don't use sample 0 and only first SAMPLES/2 are usable. Each array eleement represents a frequency and its value the amplitude.
    if (vReal[i] > max)
    {
      max = vReal[i];
    }

    if (vReal[i] < noiseThreshold) continue;
    // Serial.print(i);
    // Serial.print(":");
    // Serial.print((int)(vReal[i]));
    // Serial.print(",");

    for (byte u = 0; u < bandCount; u++)
    {
      if (i <= bandIndices[u])
      {
        updateBand(u, (int)(vReal[i] / amplitude));
        break;
      }
    }
  }

  //Serial.println();
  /*Serial.print("Update");
  Serial.println(micros() - time);
  time = micros();*/

/*
  printVector(vReal, (samples >> 1), SCL_FREQUENCY);
  float x = FFT.majorPeak();
  Serial.print("f0=");
  Serial.print(x, 6);
  Serial.println("Hz");*/

  for (byte band = 0; band < bandCount; band++) 
  {
    printBand(band, peak[band]);
  }

  //if (millis()%4 == 0) {for (byte band = 0; band <= 6; band++) {if (peak[band] > 0) peak[band] -= 1;}} // Decay the peak



  /*Serial.print("Print");
  Serial.println(micros() - time);*/
  
  processBeat(peak[1] > beatPeakThreshold || peak[2] > beatPeakThreshold);

  if (isInBeat)
  {
    Serial.print(" [beat] ");
  }

  Serial.print(max);
  Serial.println();

  for (byte u = 0; u < bandCount; u++)
  {
    peak[u] = 0;
  }
}

void processBeat(bool isBeat)
{
  long time = millis();

  if (time - lastBeatTimeMs <= beatSignalDurationMs) return;
  
  if (isBeat != isInBeat)
  {
    if (isBeat)
    {
      if (time - lastBeatTimeMs > minBeatIntervalMs)
      {
        lastBeatTimeMs = time;
        setIsInBeat(true);
        Serial.print("BEAT ");
      }
      
    }
    else
    {
      setIsInBeat(false);
    }
  }
}

void setIsInBeat(bool isBeat)
{
  isInBeat = isBeat;
  digitalWrite(LED_PIN, isBeat ? HIGH : LOW);
  digitalWrite(BEAT_PIN, isBeat ? HIGH : LOW);
}

void updateBand(int band, int dsize){
  if (dsize > bandMaxValue) {
    dsize = bandMaxValue;
  }

  if (dsize > peak[band]) {
    peak[band] = dsize;
  }
}

void printBand(int band, int peak)
{
  Serial.print("[");

  // Format the number with leading spaces (total width of 4 characters, including the number itself)
  sprintf(printBuffer, "%4d", peak);

  Serial.print(printBuffer);
  Serial.print("] ");
}


void printVector(double *vData, uint16_t bufferSize, uint8_t scaleType)
{
  for (uint16_t i = 0; i < bufferSize; i++)
  {
    double abscissa;
    /* Print abscissa value */
    switch (scaleType)
    {
      case SCL_INDEX:
        abscissa = (i * 1.0);
  break;
      case SCL_TIME:
        abscissa = ((i * 1.0) / samplingFrequency);
  break;
      case SCL_FREQUENCY:
        abscissa = ((i * 1.0 * samplingFrequency) / samples);
  break;
    }
    Serial.print(abscissa, 6);
    if(scaleType==SCL_FREQUENCY)
      Serial.print("Hz");
    Serial.print(" ");
    Serial.println(vData[i], 4);
  }
  Serial.println();
}
