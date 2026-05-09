/*
====================================
=         MAIN TEST MPU6050        =
====================================
*/

#include "MPU6050_clone.h"

MPU6050_clone mpu;

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
    mpu.setAccRange(MPU6050_clone::AFS_SEL_4G);
    mpu.setGyroRange(MPU6050_clone::FS_SEL_2000);
}

void loop()
{
    mpu.readGAT();
    print();
    delay(200);
}