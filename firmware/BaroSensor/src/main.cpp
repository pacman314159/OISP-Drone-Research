#include <Arduino.h>
#include <Wire.h>
#include "GY87_BMP180.h"

const int numSamples = 500;

float barometerAltitude(int32_t pressure) {
  return 44330.0 * (1.0 - pow((pressure / 101325.0), (1.0 / 5.255))) * 100.0;
}

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000);
  delay(10);

  readCalibrationData();

  int32_t samples[numSamples];
  uint32_t timeInst[numSamples] = {0};

  for(int i = 0; i < numSamples; i++){
    samples[i] = getPressure();
    timeInst[i] = millis(); 
  }

  for(int i = 0; i < numSamples; i+= 1)
    Serial.printf("%d, %d\n", timeInst[i], samples[i]);

}

void loop() {}
