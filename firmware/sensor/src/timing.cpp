#include <Arduino.h>
#include <Wire.h>
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
    Serial.print(mpu.getTemp());

    Serial.println();
}

const uint16_t NUM_READS = 1000;
// Store data
float AccX_Buffer[NUM_READS];
float AccY_Buffer[NUM_READS];
float AccZ_Buffer[NUM_READS];

float GyroX_Buffer[NUM_READS];
float GyroY_Buffer[NUM_READS];
float GyroZ_Buffer[NUM_READS];

float Temp_Buffer[NUM_READS];

uint32_t startTime;
uint32_t endTime;


void setup()
{
    Serial.begin(115200);
    while(!Serial);
    delay(2000);
    Wire.setClock(400000);
    mpu.initialize(10, 9);
    mpu.enableBypass();
    mpu.setAccRange(AFS_SEL_4G);
    mpu.setGyroRange(FS_SEL_2000);
    delay(100);


    //------MEASURE DURATION--------//
    startTime = micros();
     for(int i = 0; i < NUM_READS; i++)
    {
    mpu.getRawAll();
 
        AccX_Buffer[i] = mpu.getAccX();
        AccY_Buffer[i] = mpu.getAccY();
        AccZ_Buffer[i] = mpu.getAccZ();
        GyroX_Buffer[i] = mpu.getGyroX();
        GyroY_Buffer[i] = mpu.getGyroY();
        GyroZ_Buffer[i] = mpu.getGyroZ();
        Temp_Buffer[i] = mpu.getTemp();
    }
    endTime =micros();

    //-------------PRINT AFTER TIMING------------//
    Serial.println();
    Serial.println("========== RESULT ==========");

    Serial.print("Total Reads: ");
    Serial.println(NUM_READS);

    Serial.print("Total Duration (us): ");
    Serial.println(endTime - startTime);

    Serial.print("Average Per Read (us): ");

    Serial.println((float)(endTime - startTime) / NUM_READS);

    Serial.println();

    Serial.println();

    // ===== PRINT DATA =====
    for(int i = 0; i < NUM_READS; i++)
    {
        Serial.print(i);

        Serial.print(" | Acc: ");
        Serial.print(AccX_Buffer[i], 7);
        Serial.print(", ");
        Serial.print(AccY_Buffer[i], 7);
        Serial.print(", ");
        Serial.print(AccZ_Buffer[i], 7);

        Serial.print(" | Gyro: ");
        Serial.print(GyroX_Buffer[i], 7);
        Serial.print(", ");
        Serial.print(GyroY_Buffer[i], 7);
        Serial.print(", ");
        Serial.print(GyroZ_Buffer[i], 7);

        Serial.print(" | Temp: ");
        Serial.println(Temp_Buffer[i], 2);
    }

    Serial.flush();

    delay(5000);

}


void loop()
{
}