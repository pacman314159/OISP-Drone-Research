/*
====================================
=         MAIN TEST MPU6050        =
====================================
    Acc Data = AccRaw / AccSens
    GyroData = GyroRaw / GyroSens
    TempData = TempRaw / TempSens
    
    Gyro Range  | LSB Sensitivity 
    FS_SEL_250  |      131.0
    FS_SEL_500  |       65.0
    FS_SEL_1000 |       32.8
    FS_SEL_2000 |       16.4

    Acc Range   | LSB Sensitivity 
    AFS_SEL_2G  |      16384.0
    AFS_SEL_4G  |       8192.0
    AFS_SEL_8G  |       4096.0
    AFS_SEL_16G |       2048.0



*/
#include "GY87_MPU6050.h"

GY87_MPU6050 mpu;


void setup()
{
    Serial.begin(115200);

    mpu.initialize(10, 9);

    mpu.enableBypass();

    Serial.println("MPU6050 TEST");
}

void loop()
{
    mpu.getRawAll();

    mpu.getAllData(aX, aY, aZ, t, gX, gY, gZ);

    Serial.println("===== RAW =====");

    Serial.print("AccX_R: ");
    Serial.println(mpu.getRawAccX());

    Serial.print("AccY_R: ");
    Serial.println(mpu.getRawAccY());

    Serial.print("AccZ_R: ");
    Serial.println(mpu.getRawAccZ());

    Serial.print("Temp_R: ");
    Serial.println(mpu.getRawTemp());

    Serial.print("GyroX_R: ");
    Serial.println(mpu.getRawGyroX());

    Serial.print("GyroY_R: ");
    Serial.println(mpu.getRawGyroY());

    Serial.print("GyroZ_R: ");
    Serial.println(mpu.getRawGyroZ());

    Serial.println("===== REAL DATA =====");

    Serial.print("AccX(g): ");
    Serial.println(aX);

    Serial.print("AccY(g): ");
    Serial.println(aY);

    Serial.print("AccZ(g): ");
    Serial.println(aZ);

    Serial.print("Temp(C): ");
    Serial.println(t);

    Serial.print("GyroX(deg/s): ");
    Serial.println(gX);

    Serial.print("GyroY(deg/s): ");
    Serial.println(gY);

    Serial.print("GyroZ(deg/s): ");
    Serial.println(gZ);

    Serial.println();

    delay(500);
}