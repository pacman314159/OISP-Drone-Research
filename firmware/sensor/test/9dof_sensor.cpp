#include <Arduino.h>
#include <Wire.h>
#include "GY87_MPU6050.h"
#include "GY87_HMC5883L.h"

#define SDA_PIN 10
#define SCL_PIN 9
#define DRDY_PIN 7

const uint16_t SAMPLING_INTERVAL_US = 200;

GY87_MPU6050 mpu;
GY87_HMC5883L hmc;

float gx, gy, gz;
float ax, ay, az;
float mx, my, mz;
float temp;

uint32_t startTimeUs;

void setup(){
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000);

  mpu.init(SDA_PIN, SCL_PIN);
  mpu.enableBypass();
  mpu.setAccRange(AFS_SEL_4G);
  mpu.setGyroRange(FS_SEL_500);

  hmc.init(SDA_PIN, SCL_PIN);
  hmc.setMeasMode(SINGLE_MODE);
  hmc.setAveraging(AVERAGING_1);
  hmc.setOutputRate(RATE_75);
  hmc.setBiasMode(BIAS_NORMAL);
  hmc.setGain(FIELD_RANGE_0_88);
  hmc.attachDRDYInterrupt(DRDY_PIN);

  delay(100);
}

void loop(){
  startTimeUs = micros();

  mpu.getRawAll();
  hmc.getRawAll();
  hmc.setMeasMode(SINGLE_MODE);

  mpu.getAllData(ax, ay, az, temp, gx, gy, gz);
  hmc.getAllData(mx, my, mz)

  while(micros() - startTimeUs < SAMPLING_INTERVAL_US) yield();
}

