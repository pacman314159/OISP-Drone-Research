//MPU6050.h
#ifndef GY87_MPU6050_H
#define GY87_MPU6050_H



#include <Arduino.h>
#include <Wire.h>

// #define MPU6050_I2CADDR_DEFAULT 0x68 ///< MPU6050 default i2c address w/ AD0 low
// #define MPU6050_DEVICE_ID 0x68       ///< The correct MPU6050_WHO_AM_I value


// #define Mpu6050 0x68
// #define MPU6050_WHO_AM_I 0x75
// #define MPU6050_INT_PIN_CFG 0x37
// #define MPU6050_PWR_MGMT_1 0x6B    
// #define MPU6050_USER_CTRL 0x6A

// #define MPU6050_GYRO_CONFIG 0x1B
// #define MPU6050_ACCEL_CONFIG 0x1C


// #define MPU6050_GYRO_XOUT_H 0x43
// #define MPU6050_ACCEL_XOUT_H 0x3B

class GY87_MPU6050
{
    public:
        //Constructor
        GY87_MPU6050(uint8_t = 0x68);
        void begin(uint8_t sda, uint8_t scl);
        enum GyroRange 
        {
            FS_SEL_250 = 0,
            FS_SEL_500 = 1,
            FS_SEL_1000 = 2,
            FS_SEL_2000 = 3
        };
        enum AccRange 
        {
            AFS_SEL_2G = 0,
            AFS_SEL_4G = 1,
            AFS_SEL_8G = 2,
            AFS_SEL_16G = 3
        };
        void enableBypass();
        void setGyroRange(GyroRange range);
        void setAccRange(AccRange range);

        void readGAT();

        float getGyroX();
        float getGyroY();
        float getGyroZ();

        float getAccX();
        float getAccY();
        float getAccZ();

        float getTemp();

    private:
        uint8_t _addr;
        const uint8_t PWR_MGMT_1 = 0x6B;
        const uint8_t GYRO_CONFIG = 0x1B;
        const uint8_t ACCEL_CONFIG = 0x1C;
        const uint8_t ACCEL_XOUT_H = 0x3B;
        const uint8_t USER_CTRL = 0x6A;
        const uint8_t INT_PIN_CFG = 0X37;

        int16_t accX_raw, accY_raw, accZ_raw;
        int16_t gyroX_raw, gyroY_raw, gyroZ_raw;
        int16_t temp_raw;

        float accX, accY, accZ;
        float gyroX, gyroY, gyroZ;
        float temp;

        float acc_sens;
        float gyro_sens;

        void writeRegister(uint8_t reg, uint8_t data);
        uint8_t readRegister(uint8_t reg);

    
};





#endif
