#include <Filters.h>
#include "arduinoFFT.h"
#include <Adafruit_NeoPixel.h>

arduinoFFT FFT = arduinoFFT(); /* Create FFT object */
/*
These values can be changed in order to evaluate the functions
*/
#define MIC_PIN A0
#define LED_PIN 2
#define N_LEDS 30

// switch this its wrong
float highfilterFrequency = 200.0; 
FilterOnePole highpassFilter( HIGHPASS, highfilterFrequency );
float lowfilterFrequency = 20.0; 
FilterOnePole lowpassFilter( LOWPASS, lowfilterFrequency );  

Adafruit_NeoPixel strip = Adafruit_NeoPixel(N_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
const uint16_t samples = 32; //This value MUST ALWAYS be a power of 2
const double samplingFrequency = 500; //Hz, must be less than 10000 due to ADC

unsigned int sampling_period_us;
unsigned long microseconds;

/*
These are the input and output vectors
Input vectors receive computed results from FFT
*/
double vReal[samples];
double vImag[samples];

#define SCL_INDEX 0x00
#define SCL_TIME 0x01
#define SCL_FREQUENCY 0x02
#define SCL_PLOT 0x03

void setup()
{
  sampling_period_us = round(1000000*(1.0/samplingFrequency));
  Serial.begin(115200);
  strip.begin();
  for (int i = 29; i >= 0; i--)
  {
     strip.setPixelColor(i, 255, 0,255);
     strip.setPixelColor(29-i, 255, 0, 255);
     strip.show();
     delay(60);
  }
  Serial.println("Ready");
}

void loop()
{
  double low = 0;
  double mid = 0;
  double high = 0;
  /*SAMPLING - improve!!!!!!!*/
  microseconds = micros();
  for(int i=0; i<samples; i++)
  {
      float x = lowpassFilter.input(analogRead(MIC_PIN));
      float filtered_x = highpassFilter.input(x);
      vReal[i] = filtered_x;
      vImag[i] = 0;
      while(micros() - microseconds < sampling_period_us){
        //empty loop
      }
      microseconds += sampling_period_us;
  }

  FFT.Windowing(vReal, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);	/* Weigh data */
  FFT.Compute(vReal, vImag, samples, FFT_FORWARD); /* Compute FFT */ 
  FFT.ComplexToMagnitude(vReal, vImag, samples); /* Compute magnitudes */
//  for(int i=0; i<(samples/2); i++)
//  {
//    if (i<=3 )             low += vReal[i];
//    if (i >3   && i<=12 )  mid += vReal[i];
//    if (i >12)             high += vReal[i];
//  };
  ShowColors(vReal);
  //Print(vReal, samples);

  Serial.print(vReal[3]);
  Serial.print(" ");
  Serial.print(vReal[4]);
  Serial.print(" ");
  Serial.println(vReal[5]);

  //delay(1);
}


void ShowColors(double *vReal)
{
  uint32_t col = strip.Color(255, 0, 255);

  
  if (vReal[4]>=4) col = strip.Color(0, 0, 255);
  if (vReal[4]>=21) col = strip.Color(0, 255, 255);
  
  for (int i = 0; i <= 30; i++)
  {
     strip.setPixelColor(i, col);
  };

  strip.show();

}

void ShowAllColors(double *vReal){
  uint32_t col = strip.Color(255, 0, 255);
  uint32_t change = strip.Color(0, 0, 255);
  for (int i = 1; i <= 15; i++)
  {
    if (vReal[i]>=150){
      strip.setPixelColor((i*2)-2, change);
      strip.setPixelColor((i*2)-1, change);
      } else {
        strip.setPixelColor((i*2)-2, col);
        strip.setPixelColor((i*2)-1, col);
        };
        strip.show();
  };
}

void Print(double *vData, uint16_t samples)
{
  for (uint16_t i = 0; i < (samples/2); i++)
  {
    double f;
    f = ((i * 1.0 * (samplingFrequency/2)) / (samples/2));
    Serial.print(f);
    Serial.print("Hz");
    Serial.print(" ");
    Serial.println(vData[i], 4);
  }
  Serial.println();
}
