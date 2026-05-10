#include "GY87_MPU6050.h"




// Constructor
GY87_MPU6050::GY87_MPU6050(uint8_t address)
{
    _addr = address;
    accSens  = 16384.0; // [default] ±2g
    gyroSens = 131.0;   // [default] ±250 deg/s
}

//==========   QUES 1   ==========//
// Ê cái đóng này nè là tui éo muốn khai báo aX,aY,aZ,... trong main, nên tui thêm đóng này vô nhm GPT bảo là làm z nhìn kì
// Nên là check dùm tui 
    //1. Tui k muốn khai báo float trong main, idea này cần thiết k? 
    //2. Nếu idea 1 oke thì cách này oke k hay kiếm cách khác?
float aX = 0;
float aY = 0;
float aZ = 0;

float gX = 0;
float gY = 0;
float gZ = 0;

float t = 0;
//==========   QUES 1   ==========//





//==================================
//    WRITE/ READ REGISTER FUNCTION
//==================================
void GY87_MPU6050::writeRegister(uint8_t reg, uint8_t data)
{
    Wire.beginTransmission(_addr);
    Wire.write(reg);
    Wire.write(data);
    Wire.endTransmission();
}

uint8_t GY87_MPU6050::readRegister(uint8_t reg)
{
    Wire.beginTransmission(_addr);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom(_addr, (uint8_t)1);
    if(Wire.available()){
        return Wire.read();
    }
    return 0;
}

//==================================
//              FUNCTION
//==================================

//------   SETUP FUNCTION   ------//
void GY87_MPU6050::initialize(uint8_t sda, uint8_t scl)
{
    Wire.begin(sda, scl);
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
    Wire.beginTransmission(_addr);
    Wire.write(ACCEL_XOUT_H);
    Wire.endTransmission(false);
    Wire.requestFrom(_addr, (uint8_t)14);
    if(Wire.available() >= 14)
{
        aX_R = (Wire.read() << 8) | Wire.read();
        aY_R = (Wire.read() << 8) | Wire.read();
        aZ_R = (Wire.read() << 8) | Wire.read();
        t_R = (Wire.read() << 8) | Wire.read();
        gX_R = (Wire.read() << 8) | Wire.read();
        gY_R = (Wire.read() << 8) | Wire.read();
        gZ_R = (Wire.read() << 8) | Wire.read();
}
}
//==========   QUES 2    ==========//
// Ê cái đóng này nè tui add vô để print mà không tạo struct á. Lowkey không biết cần hay k:)
int16_t GY87_MPU6050::getRawAccX()
{
    return aX_R;
}

int16_t GY87_MPU6050::getRawAccY()
{
    return aY_R;
}

int16_t GY87_MPU6050::getRawAccZ()
{
    return aZ_R;
}

int16_t GY87_MPU6050::getRawGyroX()
{
    return gX_R;
}

int16_t GY87_MPU6050::getRawGyroY()
{
    return gY_R;
}

int16_t GY87_MPU6050::getRawGyroZ()
{
    return gZ_R;
}

int16_t GY87_MPU6050::getRawTemp()
{
    return t_R;
}
//==========   QUES 2   ==========//

float GY87_MPU6050::getAccX()
{
    aX = (float)aX_R / accSens;
    return aX;
}

float GY87_MPU6050::getAccY()
{
    aY = (float)aY_R / accSens;
    return aY;
}

float GY87_MPU6050::getAccZ()
{
    aZ = (float)aZ_R / accSens;
    return aZ;
}

float GY87_MPU6050::getTemp()
{
    t = (float)t_R / 340.0f + 36.53f;
    return t;
}

float GY87_MPU6050::getGyroX()
{
    gX = (float)gX_R / gyroSens;
    return gX;
}

float GY87_MPU6050::getGyroY()
{
    gY = (float)gY_R / gyroSens;
    return gY;
}

float GY87_MPU6050::getGyroZ()
{
    gZ = (float)gZ_R / gyroSens;
    return gZ;
}

void GY87_MPU6050::getAllData(float &aX, float &aY, float &aZ, float &t, float &gX, float &gY, float &gZ)
{
    aX = getAccX();
    aY = getAccY();
    aZ = getAccZ();
    t = getTemp();
    gX = getGyroX();
    gY = getGyroY();
    gZ = getGyroZ();
}
