#include "GY87_HMC5883L.h"

GY87_HMC5883L::GY87_HMC5883L(){
  sensitivity = DEFAULT_GAIN_LSB_PER_GAUSS;
}

void GY87_HMC5883L::setAveraging(AvgMode avg){
  uint8_t reg;
  reg = readRegister(HMC5883L_CRA);
  switch(avg)
  {
    case AVERAGING_1:
      reg &= ~((1 << 6) | (1 << 5));
      break;
    case AVERAGING_2:
      reg &= ~(1 << 6);
      reg |=  (1 << 5);
      break;
    case AVERAGING_4:
      reg &= ~(1 << 5);
      reg |=  (1 << 6);
      break;
    case AVERAGING_8:
      reg |= (1 << 6) | (1 << 5);
      break;
    default:
      reg &= ~((1 << 6) | (1 << 5));
      break;
  }
  writeRegister(HMC5883L_CRA, reg);
}

void GY87_HMC5883L::setOutputRate(OutputRate rate){
  uint8_t reg;
  reg = readRegister(HMC5883L_CRA);
  switch(rate)
  {
    case RATE_0_75:
      reg &= ~((1 << 4) | (1 << 3) | (1 << 2));
      break;
    case RATE_1_5:
      reg &= ~((1 << 4) | (1 << 3));
      reg |=  (1 << 2);
      break;
    case RATE_3:
      reg &= ~((1 << 4) | (1 << 2));
      reg |=  (1 << 3);
      break;
    case RATE_7_5:
      reg &= ~(1 << 4);
      reg |=  (1 << 3) | (1 << 2);
      break;
    case RATE_15:
      reg &= ~((1 << 3) | (1 << 2));
      reg |=  (1 << 4);
      break;
    case RATE_30:
      reg &= ~(1 << 3);
      reg |=  (1 << 4) | (1 << 2);
      break;
    case RATE_75:
      reg &= ~(1 << 2);
      reg |=  (1 << 4) | (1 << 3);
      break;
    default:
      reg &= ~((1 << 4) | (1 << 3) | (1 << 2)); // RATE 15
      break;
  }
  writeRegister(HMC5883L_CRA, reg);
}

void GY87_HMC5883L::setBiasMode(BiasMode bias){
  uint8_t reg;
  reg = readRegister(HMC5883L_CRA);
  switch(bias)
  {
    case BIAS_NORMAL:
      reg &= ~((1 << 1) | (1 << 0));
      break;
    case BIAS_POSITIVE:
      reg &= ~(1 << 1);
      reg |=  (1 << 0);
      break;
    case BIAS_NEGATIVE:
      reg &= ~(1 << 0);
      reg |=  (1 << 1);
      break;
    default:
      reg &= ~((1 << 1) | (1 << 0));
      break;
  }
  writeRegister(HMC5883L_CRA, reg);
}

void GY87_HMC5883L::setGain(GainMode gain){
  uint8_t reg;
  reg = readRegister(HMC5883L_CRB);
  switch(gain)
  {
    case FIELD_RANGE_0_88:
      reg &= ~((1 << 7) | (1 << 6) | (1 << 5));
      sensitivity = 1370.0;
      break;
    case FIELD_RANGE_1_3:
      reg &= ~((1 << 7) | (1 << 6));
      reg |=  (1 << 5);
      sensitivity = 1090.0;
      break;
    case FIELD_RANGE_1_9:
      reg &= ~((1 << 7) | (1 << 5));
      reg |=  (1 << 6);
      sensitivity = 820.0;
      break;
    case FIELD_RANGE_2_5:
      reg &= ~(1 << 7);
      reg |=  (1 << 6) | (1 << 5);
      sensitivity = 660.0;
      break;
    case FIELD_RANGE_4_0:
      reg &= ~(1 << 6);
      reg |=  (1 << 7) | (1 << 5);
      sensitivity = 440.0;
      break;
    case FIELD_RANGE_4_7:
      reg &= ~(1 << 5);
      reg |=  (1 << 7) | (1 << 6);
      sensitivity = 390.0;
      break;
    case FIELD_RANGE_5_6:
      reg |= (1 << 7) | (1 << 6);
      reg &= ~(1 << 5);
      sensitivity = 330.0;
      break;
    case FIELD_RANGE_8_1:
      reg |= (1 << 7) | (1 << 6) | (1 << 5);
      sensitivity = 230.0;
      break;
    default:
      reg &= ~((1 << 7) | (1 << 6));
      reg |=  (1 << 5);
      sensitivity = DEFAULT_GAIN_LSB_PER_GAUSS;
      break;
  }
  writeRegister(HMC5883L_CRB, reg);
}

void GY87_HMC5883L::setMeasMode(MeasMode meas){
  uint8_t reg;
  reg = readRegister(HMC5883L_MODE);
  switch(meas)
  {
    case CONTINUOUS_MODE:
      reg &= ~((1 << 1) | (1 << 0));
      break;
    case SINGLE_MODE:
      reg &= ~(1 << 1);
      reg |=  (1 << 0);
      break;
    case IDLE_MODE:
      reg &= ~(1 << 0);
      reg |=  (1 << 1);
      break;
    default:
      reg &= ~((1 << 1) | (1 << 0));
      break;
  }
  writeRegister(HMC5883L_MODE, reg);
}

void GY87_HMC5883L::getRawAll(){
  const uint8_t readLength = 6;
  Wire.beginTransmission(HMC5883L_ADDRESS);
  Wire.write(HMC5883L_DATA_X_MSB);
  Wire.endTransmission(false);
  Wire.requestFrom((uint8_t)HMC5883L_ADDRESS, (size_t)readLength, true);

  xRaw = (Wire.read() << 8) | Wire.read();
  zRaw = (Wire.read() << 8) | Wire.read();
  yRaw = (Wire.read() << 8) | Wire.read();
  Serial.println(xRaw, BIN);
}

int16_t GY87_HMC5883L::getRawX(){
  return xRaw;
}

int16_t GY87_HMC5883L::getRawY(){
  return yRaw;
}

int16_t GY87_HMC5883L::getRawZ(){
  return zRaw;
}


void GY87_HMC5883L::getAllData(float &_x, float &_y, float &_z){
  _x = getX();
  _y = getY();
  _z = getZ();
}

float GY87_HMC5883L::getX(){
  x = xRaw / sensitivity;
  return x;
}

float GY87_HMC5883L::getY(){
  y = yRaw / sensitivity;
  return y;
}

float GY87_HMC5883L::getZ(){
  z = zRaw / sensitivity;
  return z;
}

void GY87_HMC5883L::writeRegister(uint8_t reg, uint8_t data){
  Wire.beginTransmission(HMC5883L_ADDRESS);
  Wire.write(reg);
  Wire.write(data);
  Wire.endTransmission();
}

uint8_t GY87_HMC5883L::readRegister(uint8_t reg){
  Wire.beginTransmission(HMC5883L_ADDRESS);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom((uint8_t)HMC5883L_ADDRESS, (size_t)1, true);
  return Wire.read();
}
