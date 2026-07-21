#include "sensor/GY87_MPU6050.h"

// Constructor
GY87_MPU6050::GY87_MPU6050()
{
  // add inverse sens here, later do mult instead of div -> faster
  accSens  = DEFAULT_ACC_SENS;  
  gyroSens = DEFAULT_GYRO_SENS;
}

//==================================
//    WRITE/ READ REGISTER FUNCTION
//==================================
void GY87_MPU6050::writeRegister(uint8_t reg, uint8_t data)
{
  Wire.beginTransmission(GY87_MPU6050_ADDR);
  Wire.write(reg);
  Wire.write(data);
  Wire.endTransmission();
}

uint8_t GY87_MPU6050::readRegister(uint8_t reg)
{
  Wire.beginTransmission(GY87_MPU6050_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom((uint8_t)GY87_MPU6050_ADDR, (size_t)1);
  if(Wire.available()){
    return Wire.read();
  }
  return 0;
}

//==================================
//              FUNCTION
//==================================

//------   SETUP FUNCTION   ------//
void GY87_MPU6050::init(uint8_t sda, uint8_t scl)
{
  Wire.begin(sda, scl);
  Wire.setClock(400000);
  enableSleep(false);
  setClockSource(CLOCK_PLL_XGYRO);
  setGyroRange(FS_SEL_250);
  setAccRange(AFS_SEL_2G);
}

void GY87_MPU6050::enableSleep(bool enable){
  uint8_t reg = readRegister(PWR_MGMT_1);
  if(enable){
    reg |= (1 << 6);
  }
  else{
    reg &= ~(1 << 6);
  }
  writeRegister(PWR_MGMT_1, reg);
}

void GY87_MPU6050::setClockSource(ClockSource clocksource){
  uint8_t reg = readRegister(PWR_MGMT_1);
  switch(clocksource)
  {
    case CLOCK_INTERNAL: //000
      reg &= ~((1 << 0) | (1 << 1) | (1 << 2));
      break;
    case CLOCK_PLL_XGYRO: //001
      reg |= (1<<0);
      reg &= ~((1 << 1) | (1 << 2));
      break;
    case CLOCK_PLL_YGYRO: //010
      reg |= (1<<1);
      reg &= ~((1 << 0) | (1 << 2));
      break;    
    case CLOCK_PLL_ZGYRO: //011
      reg |= ((1 << 0) | (1 << 1));
      reg &= ~(1<<2);
      break;
    case CLOCK_PLL_EXT32_768_KHz:  //100
      reg |= (1<<2);
      reg &= ~((1 << 0) | (1 << 1));
      break;
    case CLOCK_PLL_EXT_19_2MHz:  //101
      reg |= ((1 << 0) | (1 << 2));
      reg &= ~(1<<1);
      break;
    case MPU6050_CLOCK_KEEP_RESET:  //111  
      reg |= ((1 << 0) | (1 << 1) | (1 << 2));
      break;
  }
  writeRegister(PWR_MGMT_1, reg);
}

void GY87_MPU6050::setGyroRange(GyroRange range)
{
  uint8_t reg = readRegister(GYRO_CONFIG);
  switch(range)
  {
    case FS_SEL_250:  
      reg &= ~((1 << 4) | (1 << 3));
      gyroSens = 131.0; 
      break;
    case FS_SEL_500:  
      reg &= ~(1 << 4);
      reg |= (1 <<3);
      gyroSens = 65.5;  
      break;
    case FS_SEL_1000: 
      reg |= (1 << 4);
      reg &= ~(1 <<3);
      gyroSens = 32.8;  
      break;
    case FS_SEL_2000: 
      reg |= ((1 << 4) | (1 << 3));
      gyroSens = 16.4;  
      break;
  }
  writeRegister(GYRO_CONFIG, reg);
}

void GY87_MPU6050::setAccRange(AccRange range)
{
  uint8_t reg = readRegister(ACCEL_CONFIG);
  switch(range)
  {
    case AFS_SEL_2G: 
      reg &= ~((1 << 4) | (1 << 3));
      accSens = 16384.0; 
      break;
    case AFS_SEL_4G: 
      reg &= ~(1 << 4);
      reg |= (1 <<3);
      accSens = 8192.0; 
      break;
    case AFS_SEL_8G:
      reg |= (1 << 4);
      reg &= ~(1 <<3);
      accSens = 4096.0; 
      break;
    case AFS_SEL_16G: 
      reg |= ((1 << 4) | (1 << 3));
      accSens = 2048.0; 
      break;
  }
  writeRegister(ACCEL_CONFIG, reg);
}

void GY87_MPU6050::enableBypass(){
  uint8_t reg1 = readRegister(USER_CTRL);
  reg1 &=~ (1<<5);
  writeRegister(USER_CTRL,reg1);

  uint8_t reg2 = readRegister(INT_PIN_CFG);
  reg2 |= (1 << 1);
  writeRegister(INT_PIN_CFG,reg2);
}

//------   DATA FUNCTION   ------//
void GY87_MPU6050::getRawAll()
{
  const uint8_t readLength = 14; // prevent magic number

  Wire.beginTransmission(GY87_MPU6050_ADDR);
  Wire.write(ACCEL_XOUT_H);
  Wire.endTransmission(false);
  Wire.requestFrom((uint8_t)GY87_MPU6050_ADDR, (size_t)readLength);
  if(Wire.available() >= readLength)
  {
    aXRaw = (Wire.read() << 8) | Wire.read();
    aYRaw = (Wire.read() << 8) | Wire.read();
    aZRaw = (Wire.read() << 8) | Wire.read();
    tempRaw = (Wire.read() << 8) | Wire.read();
    gXRaw = (Wire.read() << 8) | Wire.read();
    gYRaw = (Wire.read() << 8) | Wire.read();
    gZRaw = (Wire.read() << 8) | Wire.read();
  }
}

int16_t GY87_MPU6050::getRawAccX()
{
  return aXRaw;
}

int16_t GY87_MPU6050::getRawAccY()
{
  return aYRaw;
}

int16_t GY87_MPU6050::getRawAccZ()
{
  return aZRaw;
}

int16_t GY87_MPU6050::getRawGyroX()
{
  return gXRaw;
}

int16_t GY87_MPU6050::getRawGyroY()
{
  return gYRaw;
}

int16_t GY87_MPU6050::getRawGyroZ()
{
  return gZRaw;
}

int16_t GY87_MPU6050::getRawTemp()
{
  return tempRaw;
}

float GY87_MPU6050::getAccX()
{
  aX = aXRaw / accSens;
  return aX;
}

float GY87_MPU6050::getAccY()
{
  aY = aYRaw / accSens;
  return aY;
}

float GY87_MPU6050::getAccZ()
{
  aZ = aZRaw / accSens;
  return aZ;
}

float GY87_MPU6050::getTemp()
{
  temp = tempRaw / 340.0f + 36.53f;
  return temp;
}

float GY87_MPU6050::getGyroX()
{
  gX = gXRaw / gyroSens;
  return gX;
}

float GY87_MPU6050::getGyroY()
{
  gY = gYRaw / gyroSens;
  return gY;
}

float GY87_MPU6050::getGyroZ()
{
  gZ = gZRaw / gyroSens;
  return gZ;
}

void GY87_MPU6050::getAllData(float &_aX, float &_aY, float &_aZ, float &_t, float &_gX, float &_gY, float &_gZ)
{
  // add "_" before variable to prevent mismatching function params with class's attributes
  _aX = getAccX();
  _aY = getAccY();
  _aZ = getAccZ();
  _t = getTemp();
  _gX = getGyroX();
  _gY = getGyroY();
  _gZ = getGyroZ();
}
