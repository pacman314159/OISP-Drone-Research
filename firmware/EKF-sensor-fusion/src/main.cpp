#include <Arduino.h>
#include <Wire.h>
#include "GY87_MPU6050.h"
#include "GY87_HMC5883L.h"

#define SDA_PIN 10
#define SCL_PIN 9
#define DRDY_PIN 7

const uint16_t NUM_READS = 5000;
const uint16_t SAMPLING_INTERVAL_US = 230;

GY87_MPU6050 mpu;
GY87_HMC5883L hmc;

float accXSamples[NUM_READS] = {0},  accYSamples[NUM_READS] {0},    accZSamples[NUM_READS] = {0};
float gyroXSamples[NUM_READS] = {0}, gyroYSamples[NUM_READS] = {0}, gyroZSamples[NUM_READS] = {0};
float magXSamples[NUM_READS] = {0},  magYSamples[NUM_READS] = {0},  magZSamples[NUM_READS] = {0};
float tempSamples[NUM_READS]  = {0};

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

  //------MEASURE DURATION (Nhien's version)--------//
  startTimeUs = micros();
  for(int i = 0; i < NUM_READS; i++){
    // mpu.getRawAll();
    // mpu.getAllData(
    //   accXSamples[i], accYSamples[i], accZSamples[i],
    //   tempSamples[i],
    //   gyroXSamples[i], gyroYSamples[i], gyroZSamples[i]
    // );

    while(not hmc.isDataReady()) yield();
    hmc.getRawAll();
    hmc.setMeasMode(SINGLE_MODE);
    hmc.getAllData(magXSamples[i], magYSamples[i], magZSamples[i]);

    timeInstances[i] = hmc.getLatestDataTimeUs();
    delayMicroseconds(SAMPLING_INTERVAL_US);
  }
  endTime = micros();

}

void loop(){
}

