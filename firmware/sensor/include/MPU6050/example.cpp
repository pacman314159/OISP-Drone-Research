/*
====================================
=         MAIN TEST MPU6050        =
====================================
*/

#include "GY87_MPU6050.h"

GY87_MPU6050 mpu;

void print()
{
    Serial.print("Gyro: ");
    Serial.print(mpu.getGyroX()); Serial.print(", ");
    Serial.print(mpu.getGyroY()); Serial.print(", ");
    Serial.print(mpu.getGyroZ());

    Serial.print(" | Acc: ");
    Serial.print(mpu.getAccX()); Serial.print(", ");
    Serial.print(mpu.getAccY()); Serial.print(", ");
    Serial.print(mpu.getAccZ());

    Serial.print(" | Temp: ");
    Serial.println(mpu.getTemp());
}
void setup()
{
    Serial.begin(115200);

    mpu.begin(10, 9);
    mpu.enableBypass();
    mpu.setAccRange(GY87_MPU6050::AFS_SEL_4G);
    mpu.setGyroRange(GY87_MPU6050::FS_SEL_2000);
}

void loop()
{
    mpu.readGAT();
    print();
    delay(200);
}