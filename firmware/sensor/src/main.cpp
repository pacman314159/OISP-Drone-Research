#include <Arduino.h>
#include <Wire.h>
#include "GY87_MPU6050.h"
#include "GY87_HMC5883L.h"

const uint32_t NUM_READS = 100000;
const uint16_t SAMPLING_INTERVAL_US = 230;

GY87_MPU6050 mpu;
GY87_HMC5883L hmc;

// Stored data
// float accXSamples[NUM_READS] = {0},  accYSamples[NUM_READS] {0},    accZSamples[NUM_READS] = {0};
// float gyroXSamples[NUM_READS] = {0}, gyroYSamples[NUM_READS] = {0}, gyroZSamples[NUM_READS] = {0};
// float magXSamples[NUM_READS] = {0},  magYSamples[NUM_READS] = {0},  magZSamples[NUM_READS] = {0};
// float tempSamples[NUM_READS]  = {0};

uint32_t startTime, endTime;
// uint32_t timeInstances[NUM_READS] = {0};

void setup(){
  Serial.begin(115200);
  while(!Serial);
  delay(2000);

  mpu.init(10, 9);
  mpu.enableBypass();
  mpu.setAccRange(AFS_SEL_4G);
  mpu.setGyroRange(FS_SEL_2000);

  hmc.setMeasMode(SINGLE_MODE);
  hmc.setAveraging(AVERAGING_1);
  hmc.setOutputRate(RATE_75);
  hmc.setBiasMode(BIAS_NORMAL);
  hmc.setGain(FIELD_RANGE_0_88);


  delay(100);

  // //------MEASURE DURATION FOR ACCELEROMETER AND GYROSCOPE--------//
  // startTime = micros();
  // for(int i = 0; i < NUM_READS; i++){
  //   mpu.getRawAll();
  //   mpu.getAllData(
  //     accXSamples[i], accYSamples[i], accZSamples[i],
  //     tempSamples[i],
  //     gyroXSamples[i], gyroYSamples[i], gyroZSamples[i]
  //   );

  //   // hmc.setMeasMode(SINGLE_MODE);
  //   // hmc.getRawAll();
  //   // hmc.getAllData(magXSamples[i], magYSamples[i], magZSamples[i]);

  //   timeInstances[i] = micros();
  //   delayMicroseconds(SAMPLING_INTERVAL_US);
  // }
  // endTime = micros();

  //------MEASURE DURATION FOR GYROSCOPE ONLY--------//
  // startTime = micros();
  // for(int i = 0; i < NUM_READS; i++){
  //   mpu.getRawAll();
  //   mpu.getAllData(
  //     accXSamples[i], accYSamples[i], accZSamples[i],
  //     tempSamples[i],
  //     gyroXSamples[i], gyroYSamples[i], gyroZSamples[i]
  //   );

  //   // hmc.setMeasMode(SINGLE_MODE);
  //   // hmc.getRawAll();
  //   // hmc.getAllData(magXSamples[i], magYSamples[i], magZSamples[i]);

  //   timeInstances[i] = micros();
  //   delayMicroseconds(SAMPLING_INTERVAL_US);
  // }
  // endTime = micros();

  //-------------PRINT AFTER TIMING------------//
  // Serial.println();
  // Serial.println("========== RESULT ==========");
  // Serial.printf("Total Reads: %d\n", NUM_READS);
  // Serial.printf("Total Duration (us): %d\n", endTime - startTime);
  // Serial.printf("Average Per Read (us): %.4f\n", (float)(endTime - startTime) / NUM_READS);
  // Serial.printf("\n\n");

  // ===== PRINT DATA =====

  // Serial.println("timeInstance, gyroX, gyroY, gyroZ, temp");
  // for(int i = 0; i < NUM_READS; i++)
  //   Serial.printf("%d, %.6f, %.6f, %.6f, %.6f\n",
  //                 timeInstances[i],
  //                 // accXSamples[i], accYSamples[i], accZSamples[i],
  //                 gyroXSamples[i], gyroYSamples[i], gyroZSamples[i],
  //                 tempSamples[i]);

  Serial.println("timeInstance, gyroX, gyroY, gyroZ, accX, accY, accZ, temp");

  float aX, aY, aZ, t, gX, gY, gZ;

  for(uint32_t i = 0; i < NUM_READS; i++){
    mpu.getRawAll();
    mpu.getAllData(aX, aY, aZ, t, gX, gY, gZ);

    // Print immediately — no array storage needed
    Serial.printf("%lu, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f\n",
                  micros(), gX, gY, gZ, aX, aY, aZ, t);

    delayMicroseconds(SAMPLING_INTERVAL_US);
  }

//   Serial.println("timeInstance, magX, magY, magZ");
//   for(int i = 0; i < NUM_READS; i++)
//     Serial.printf("%d, %.9f, %.9f, %.9f\n",
//                   timeInstances[i],
//                   magXSamples[i], magYSamples[i], magZSamples[i]);
}

void loop(){
  // hmc.getRawAll();
  // Serial.printf("%.4f, %.4f, %.4f\n", hmc.getX(), hmc.getY(), hmc.getZ());
}

