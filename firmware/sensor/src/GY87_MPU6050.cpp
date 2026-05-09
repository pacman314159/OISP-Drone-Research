#include "GY87_MPU6050.h"

// Constructor
GY87_MPU6050::GY87_MPU6050(uint8_t address)
{
    _addr = address;
    acc_sens  = 16384.0; // ±2g
    gyro_sens = 131.0;   // ±250 deg/s
}



// Init
void GY87_MPU6050::begin(uint8_t sda, uint8_t scl)
{
    Wire.begin(sda, scl);

    // Wake up
    writeRegister(PWR_MGMT_1, 0x00);
}



// Low-level write
void GY87_MPU6050::writeRegister(uint8_t reg, uint8_t data)
{
    Wire.beginTransmission(_addr);
    Wire.write(reg);
    Wire.write(data);
    Wire.endTransmission();
}

// Low-level read
uint8_t GY87_MPU6050::readRegister(uint8_t reg)
{
    Wire.beginTransmission(_addr);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom(_addr, (uint8_t)1);
    return Wire.read();
}


    void GY87_MPU6050::enableBypass()
    {
        uint8_t reg1 = readRegister(USER_CTRL);
        reg1 &=~ (1<<5);
        writeRegister(USER_CTRL,reg1);

        uint8_t reg2 = readRegister(INT_PIN_CFG);
        reg2 |= (1 << 1);
        writeRegister(INT_PIN_CFG,reg2);

    }
// Set gyro range
void GY87_MPU6050::setGyroRange(GyroRange range)
{
    uint8_t reg = readRegister(GYRO_CONFIG);

    reg &= ~(0x18);        // clear bits 3,4
    reg |= (range << 3);   // set new

    writeRegister(GYRO_CONFIG, reg);

    switch(range)
    {
        case FS_SEL_250:  gyro_sens = 131.0; break;
        case FS_SEL_500:  gyro_sens = 65.5;  break;
        case FS_SEL_1000: gyro_sens = 32.8;  break;
        case FS_SEL_2000: gyro_sens = 16.4;  break;
    }
}

// Set accel range
void GY87_MPU6050::setAccRange(AccRange range)
{
    uint8_t reg = readRegister(ACCEL_CONFIG);

    reg &= ~(0x18);
    reg |= (range << 3);

    writeRegister(ACCEL_CONFIG, reg);

    switch(range)
    {
        case AFS_SEL_2G: acc_sens = 16384.0; break;
        case AFS_SEL_4G: acc_sens = 8192.0; break;
        case AFS_SEL_8G: acc_sens = 4096.0; break;
        case AFS_SEL_16G: acc_sens = 2048.0; break;
    }
}

// Read all data
void GY87_MPU6050::readGAT()
{
    Wire.beginTransmission(_addr);
    Wire.write(ACCEL_XOUT_H);
    Wire.endTransmission(false);
    Wire.requestFrom(_addr, (uint8_t)14);

    accX_raw = (Wire.read() << 8) | Wire.read();
    accY_raw = (Wire.read() << 8) | Wire.read();
    accZ_raw = (Wire.read() << 8) | Wire.read();

    temp_raw = (Wire.read() << 8) | Wire.read();

    gyroX_raw = (Wire.read() << 8) | Wire.read();
    gyroY_raw = (Wire.read() << 8) | Wire.read();
    gyroZ_raw = (Wire.read() << 8) | Wire.read();

    // Convert
    accX = accX_raw / acc_sens;
    accY = accY_raw / acc_sens;
    accZ = accZ_raw / acc_sens;

    gyroX = gyroX_raw / gyro_sens;
    gyroY = gyroY_raw / gyro_sens;
    gyroZ = gyroZ_raw / gyro_sens;

    temp = temp_raw / 340.0 + 36.53;
}

// Get
float GY87_MPU6050::getGyroX(){ return gyroX; }
float GY87_MPU6050::getGyroY(){ return gyroY; }
float GY87_MPU6050::getGyroZ(){ return gyroZ; }

float GY87_MPU6050::getAccX(){ return accX; }
float GY87_MPU6050::getAccY(){ return accY; }
float GY87_MPU6050::getAccZ(){ return accZ; }

float GY87_MPU6050::getTemp(){ return temp; }