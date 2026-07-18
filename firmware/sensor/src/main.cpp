#include <Arduino.h>
#include <Wire.h>
#include "GY87_MPU6050.h"
#include "GY87_HMC5883L.h"

#define SDA_PIN 10
#define SCL_PIN 9
#define DRDY_PIN 7

const uint16_t NUM_READS = 50;
const uint16_t SAMPLING_INTERVAL_US = 5000;

GY87_MPU6050 mpu;
GY87_HMC5883L hmc;

float gx, gy, gz;
float ax, ay, az;
float mx, my, mz;
float temp;

uint32_t startTimeUs, endTime;
uint32_t timeInstances[NUM_READS] = {0};

void setup(){
  Serial.begin(115200);

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
  mpu.getAllData(ax, ay, az, temp, gx, gy, gz);
  // while(not hmc.isDataReady()) yield();
  hmc.getRawAll();
  hmc.setMeasMode(SINGLE_MODE);
  hmc.getAllData(mx, my, mz);

  Serial.printf("%f, %f, %f, %f, %f, %f, %f, %f, %f\n",
                gx, gy, gz, ax, ay, az, mx, my, mz);

  while(micros() - startTimeUs < SAMPLING_INTERVAL_US) yield();
}

