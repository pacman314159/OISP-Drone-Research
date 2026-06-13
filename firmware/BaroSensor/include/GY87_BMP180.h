#ifndef GY87_BMP180_H
#define GY87_BMP180_H

#include <Arduino.h>
#include <Wire.h>

#define GY87_BMP180_ADDR 0x77
#define DEFAULT_OVERSAMPLING_SETTING 3

enum OversampSett{
  OVERSAMPLING_1,
  OVERSAMPLING_2,
  OVERSAMPLING_4,
  OVERSAMPLING_8,
};

class GY87_BMP180{
private:
  uint16_t oss;
  uint16_t ac4, ac5, ac6;
  int16_t ac1, ac2, ac3, b1, b2;
  int16_t mb, mc, md;
  int32_t up;
  long UT, UP;
  int16_t b5;

private: 
  void writeRegister(uint8_t reg, uint8_t data);
  uint8_t readRegister(uint8_t reg);

  long readUT();
  long readUP();
  int32_t calculateTT();
  int32_t calculateTP();

public:
  GY87_BMP180(){}

  void init(uint8_t sda, uint8_t scl);

  void readCalibData();

  void setOversampling(OversampSett overSamp);

  int32_t getPressurePa();

};

#endif
