/*
====================================
=         MAIN TEST MPU6050        =
====================================
*/
#include "GY87_MPU6050.h"

GY87_MPU6050 mpu;

// float aX, aY, aZ;
// float gX, gY, gZ;
// float t;

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
