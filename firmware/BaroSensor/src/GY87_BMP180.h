#include <Arduino.h>
#include <Wire.h>

#define SDA_PIN 10
#define SCL_PIN 9
#define BMP180_ADD 0x77

//Oversampling setiing
uint16_t oss = 0;

// Calibration coefficients
uint16_t ac4, ac5, ac6;        //Define unsigned short and short coefficients
int16_t ac1, ac2, ac3, b1, b2;
int16_t mb, mc, md;
int32_t up;

long UT, UP;    //Define UT and UP in global variables
int16_t b5;

/*int32_t debug_X1_P1;
int32_t debug_X2_P1;
int32_t debug_X1_P2;
int32_t debug_X2_P2;
int32_t debug_X1_P3;
int32_t debug_X2_P3;
int32_t debug_X1_P4;
int32_t debug_X1_T;
int32_t debug_X2_T;
int32_t debug_X3_P1;
int32_t debug_X3_P2;
int32_t debug_b3;
uint32_t debug_b4;
int32_t debug_b6;
int32_t press;
uint32_t debug_b7;*/

float altitudeBarometer;

void readCalibrationData(){
  Wire.beginTransmission(BMP180_ADD);
  Wire.write(0xAA);
  Wire.endTransmission(); 
  
  Wire.requestFrom(BMP180_ADD, 22);

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

long readUT(){
  Wire.beginTransmission(BMP180_ADD);
  Wire.write(0xF4);
  Wire.write(0x2E);
  Wire.endTransmission();

  delay(5);

  Wire.beginTransmission(BMP180_ADD);
  Wire.write(0xF6);
  Wire.endTransmission(false);

  Wire.requestFrom(BMP180_ADD, 2);

  if (Wire.available() < 2) {
    return -1;
  }

  uint8_t msb = Wire.read();
  uint8_t lsb = Wire.read();

  return ((long)msb << 8) | lsb;
}

long readUP(){

  Wire.beginTransmission(BMP180_ADD);
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

  delay(5);

  Wire.beginTransmission(BMP180_ADD);
  Wire.write(0xF6);
  Wire.endTransmission(false);

  Wire.requestFrom(BMP180_ADD, 3);

  if (Wire.available() < 3) {
    return -1;
  }

  uint32_t msb = Wire.read();
  uint32_t lsb = Wire.read();
  uint32_t xlsb = Wire.read();

  return (((long)msb << 16) + ((long)lsb << 8) + (long)xlsb) >> (8 - oss);
}