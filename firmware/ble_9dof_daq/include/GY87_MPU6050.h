//MPU6050.h
#ifndef GY87_MPU6050_H
#define GY87_MPU6050_H

#include <Arduino.h>
#include <Wire.h>

#define GY87_MPU6050_ADDR 0x68

const uint8_t PWR_MGMT_1 =      0x6B;
const uint8_t GYRO_CONFIG =     0x1B;
const uint8_t ACCEL_CONFIG =    0x1C;
const uint8_t ACCEL_XOUT_H =    0x3B;
const uint8_t USER_CTRL =       0x6A;
const uint8_t INT_PIN_CFG =     0X37;

const float DEFAULT_ACC_SENS =  16384.0; // [default] ±2g
const float DEFAULT_GYRO_SENS = 131.0; // [default] ±250 deg/s

enum GyroRange {
  FS_SEL_250 = 0,
  FS_SEL_500 = 1,
  FS_SEL_1000 = 2,
  FS_SEL_2000 = 3
};

enum AccRange {
  AFS_SEL_2G = 0,
  AFS_SEL_4G = 1,
  AFS_SEL_8G = 2,
  AFS_SEL_16G = 3
};

enum ClockSource {
  CLOCK_INTERNAL = 0,
  CLOCK_PLL_XGYRO = 1,
  CLOCK_PLL_YGYRO = 2,
  CLOCK_PLL_ZGYRO = 3,
  CLOCK_PLL_EXT32_768_KHz  = 4,
  CLOCK_PLL_EXT_19_2MHz  = 5,
  MPU6050_CLOCK_KEEP_RESET  = 7  
};


class GY87_MPU6050{
  // bring attributes first, methods later
private:
  // delete _addr variable because the address of this sensor never changes
  float accSens;
  float gyroSens;

  // raw data
  int16_t aXRaw, aYRaw, aZRaw;
  int16_t tempRaw;
  int16_t gXRaw, gYRaw, gZRaw;

  // converted data
  float aX, aY, aZ;
  float gX, gY, gZ;
  float temp;

  void writeRegister(uint8_t reg, uint8_t data);
  uint8_t readRegister(uint8_t reg);

public:
  GY87_MPU6050();

  void init(uint8_t sda, uint8_t scl);
  void enableSleep(bool enable);
  void enableBypass();
  void setClockSource(ClockSource source);
  void setGyroRange(GyroRange range);
  void setAccRange(AccRange range);

  void getRawAll();
  int16_t getRawAccX();
  int16_t getRawAccY();
  int16_t getRawAccZ();

  int16_t getRawGyroX();
  int16_t getRawGyroY();
  int16_t getRawGyroZ();

  int16_t getRawTemp();
  void getAllData(float &aX, float &aY, float &aZ, float &t, float &gX, float &gY, float &gZ);

  float getAccX();
  float getAccY();
  float getAccZ();

  float getTemp();

  float getGyroX();
  float getGyroY();
  float getGyroZ();

};

#endif
