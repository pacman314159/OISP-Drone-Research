#include <Arduino.h>
#include <Wire.h>
#include "GY87_BMP180.h"

const int numSamples = 5000;
int32_t samples[numSamples];
uint32_t timeInst[numSamples] = {0};

int testing = 0; //heloooooooooooo

GY87_BMP180 bmp;

float barometerAltitude(int32_t pressure) {
  return 44330.0 * (1.0 - pow((pressure / 101325.0), (1.0 / 5.255))) * 100.0;
}

void setup() {
  Serial.begin(115200);
  delay(10);

  bmp.init(10, 9);
  bmp.readCalibData();
  bmp.setOversampling(OVERSAMPLING_8);


  Serial.printf("Start retreiving %d samples\n", numSamples);
  for(int i = 0; i < numSamples; i++){
    samples[i] = bmp.getPressurePa();
    timeInst[i] = millis(); 
  }
  Serial.printf("Done\n");

  for(int i = 0; i < numSamples; i+= 1)
    Serial.printf("%d, %d\n", timeInst[i], samples[i]);

}

void loop() {}
