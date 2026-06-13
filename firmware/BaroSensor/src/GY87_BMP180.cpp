#include "GY87_BMP180.h"

void GY87_BMP180::writeRegister(uint8_t reg, uint8_t data){
  Wire.beginTransmission(GY87_BMP180_ADDR);
  Wire.write(reg);
  Wire.write(data);
  Wire.endTransmission();
}

uint8_t GY87_BMP180::readRegister(uint8_t reg){
  Wire.beginTransmission(GY87_BMP180_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom((uint8_t)GY87_BMP180_ADDR, (size_t)1);
  if(Wire.available()){
    return Wire.read();
  }
  return 0;
}

long GY87_BMP180::readUT(){
  Wire.beginTransmission(GY87_BMP180_ADDR);
  Wire.write(0xF4);
  Wire.write(0x2E);
  Wire.endTransmission();

  delay(5);

  Wire.beginTransmission(GY87_BMP180_ADDR);
  Wire.write(0xF6);
  Wire.endTransmission(false);
  Wire.requestFrom(GY87_BMP180_ADDR, 2);

  if (Wire.available() < 2) return -1;

  uint8_t msb = Wire.read();
  uint8_t lsb = Wire.read();

  UT = ((long)msb << 8) | lsb;
  return UT;
}

long GY87_BMP180::readUP(){
  Wire.beginTransmission(GY87_BMP180_ADDR);
  Wire.write(0xF4);
  Wire.write(0x34 + (oss << 6));
  Wire.endTransmission();

  switch(oss){
    case 0: delay(5); break;
    case 1: delay(8); break;
    case 2: delay(14); break;
    case 3: delay(26); break;
    default: delay(5); break;
  }

  Wire.beginTransmission(GY87_BMP180_ADDR);
  Wire.write(0xF6);
  Wire.endTransmission(false);
  Wire.requestFrom(GY87_BMP180_ADDR, 3);

  if (Wire.available() < 3) return -1;

  uint32_t msb = Wire.read();
  uint32_t lsb = Wire.read();
  uint32_t xlsb = Wire.read();

  UP = (((long)msb << 16) + ((long)lsb << 8) + (long)xlsb) >> (8 - oss);
  return UP;
}

int32_t GY87_BMP180::calculateTT() {
  int32_t X1 = ((UT - ac6) * ac5) >> 15;
  int32_t X2 = (mc << 11) / (X1 + md);
  b5 = X1 + X2;
  return (b5 + 8) >> 4;
}

int32_t GY87_BMP180::calculateTP() {
  int32_t X1, X2, X3, b3, b6, P;
  uint32_t b4;
  int64_t b7;

  b6 = b5 - 4000; 
  X1 = (b2 * ((b6 * b6) >> 12)) >> 11; 
  X2 = (ac2 * b6) >> 11; 
  X3 = X1 + X2; 

  b3 = (((ac1 * 4 + X3) << oss) + 2) / 4; 
  X1 = (ac3 * b6) >> 13; 
  X2 = (b1 * ((b6 * b6) >> 12)) >> 16; 
  X3 = ((X1 + X2) + 2) >> 2; 
  b4 = (ac4 * ((unsigned long)(X3 + 32768))) >> 15; 
  b7 = ((unsigned long)UP - b3) * (50000 >> oss); 

  if (b7 < 0x80000000) { 
    P = (b7 * 2) / b4; 
  } else { 
    P = (b7 / b4) * 2; 
  }

  X1 = (P >> 8) * (P >> 8); 
  X1 = (X1 * 3038) >> 16; 
  X2 = (-7357 * P) >> 16; 
  P = P + ((X1 + X2 + 3791) >> 4); 

  return P;
}

void GY87_BMP180::init(uint8_t sda, uint8_t scl){
  Wire.begin(sda, scl);
  Wire.setClock(400000);
  oss = DEFAULT_OVERSAMPLING_SETTING;
}

void GY87_BMP180::readCalibData(){
  Wire.beginTransmission(GY87_BMP180_ADDR);
  Wire.write(0xAA);
  Wire.endTransmission(); 
  Wire.requestFrom(GY87_BMP180_ADDR, 22);

  ac1 = (Wire.read() << 8) | Wire.read();
  ac2 = (Wire.read() << 8) | Wire.read();
  ac3 = (Wire.read() << 8) | Wire.read();
  ac4 = (Wire.read() << 8) | Wire.read();
  ac5 = (Wire.read() << 8) | Wire.read();
  ac6 = (Wire.read() << 8) | Wire.read();
  b1 = (Wire.read() << 8) | Wire.read();
  b2 = (Wire.read() << 8) | Wire.read();
  mb = (Wire.read() << 8) | Wire.read();
  mc = (Wire.read() << 8) | Wire.read();
  md = (Wire.read() << 8) | Wire.read();
}

void GY87_BMP180::setOversampling(OversampSett overSamp){
  switch(overSamp){
    case OVERSAMPLING_1:{
      oss = 0;
      break;
    }
    case OVERSAMPLING_2:{
      oss = 1;
      break;
    }
    case OVERSAMPLING_4:{
      oss = 2;
      break;
    }
    case OVERSAMPLING_8:{
      oss = 3;
      break;
    }
  }
}

int32_t GY87_BMP180::getPressurePa(){
  readUT();
  readUP();
  calculateTT(); 
  return calculateTP();
}
