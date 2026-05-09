#include <Arduino.h>
#include <Wire.h>


#define HMC5883L_ADDRESS          0x1E

// ////====================================================////
// ////                     REGISTERS                      ////
// ////====================================================////

#define HMC5883L_CRA              0x00
//  /*
//     | Location | Description                                                       |
//     | :------: | :-------------------------------------------------------------    |
//     |     0    | Set to 0 when confirguring CRA                                    |
//     |   5 - 6  | Number of samples averaged(1 to 8)                                |
//     |   2 - 4  | Data output rate(0.75 | 1.5 | 3 | 15 (default) | 30 |75 | Reverse)|
//     |   0 - 1  | Bias measurement register                                         | I dont understand this function */
#define HMC5883L_CRB              0x01
// /*
//     | Location | Description                                     |
//     | :------: | :---------------------------------------------- |
//     |   5 - 7  | Gain configuration                              |
//     |   0 - 4  | must be cleared for proper operation | */
#define HMC5883L_MODE             0x02
//  /*!< Register address for the mode register, which contains:
//     | Location | Description                                                        |
//     | :------: | :----------------------------------------------------------------- |
//     |     7    | High speed I2C mode bit                                            |
//     |   1 - 6  | Not used                                                           |
//     |   0 - 1  | Measurement mode select bits (Continuous mesurement mode | single measurement mode | idle mode)| */


#define HMC5883L_DATA_X_MSB       0x03
// /*!< Starting address for the data registers, which are, in
//      order: `DXRA` (MSB), `DXRB` (LSB), `DZRA` (MSB), 
//     `DZRB` (LSB), `DYRA` (MSB), `DYRB` (LSB). */
#define HMC5883L_DATA_X_LSB       0x04

#define HMC5883L_DATA_Z_MSB       0x05
#define HMC5883L_DATA_Z_LSB       0x06

#define HMC5883L_DATA_Y_MSB       0x07
#define HMC5883L_DATA_Y_LSB       0x08

#define HMC5883L_STATUS           0x09
// /*!< Register address for the status register, which contains
//     the `LOCK` [1] and `RDY` [0]. See `getStatus()`. */

#define HMC5883L_ID_A             0x0A
#define HMC5883L_ID_B             0x0B
#define HMC5883L_ID_C             0x0C



int16_t MagX;
int16_t MagY;
int16_t MagZ;

float Magnetic_Sensitivity = 1090.0;


enum Averaging_Mode
{
    AVERAGING_1,
    AVERAGING_2,
    AVERAGING_4,
    AVERAGING_8
};

enum Output_Rate
{
    RATE_0_75,
    RATE_1_5,
    RATE_3,
    RATE_7_5,
    RATE_15,
    RATE_30,
    RATE_75
};

enum Bias_Mode
{
    BIAS_NORMAL,
    BIAS_POSITIVE,
    BIAS_NEGATIVE
};

enum Gain_Mode
{
    FIELD_RANGE_0_88,
    FIELD_RANGE_1_3,
    FIELD_RANGE_1_9,
    FIELD_RANGE_2_5,
    FIELD_RANGE_4_0,
    FIELD_RANGE_4_7,
    FIELD_RANGE_5_6,
    FIELD_RANGE_8_1
};

enum Measurement_Mode
{
    CONTINUOUS_MODE,
    SINGLE_MODE,
    IDLE_MODE
};

// ////====================================================////
// ////               LOW LEVEL FUNCTIONS                  ////
// ////====================================================////

void HMC5883L_Write_Register(uint8_t reg, uint8_t data)
{
    Wire.beginTransmission(HMC5883L_ADDRESS);
    Wire.write(reg);
    Wire.write(data);
    Wire.endTransmission();
}

uint8_t HMC5883L_Read_Register(uint8_t reg)
{
    Wire.beginTransmission(HMC5883L_ADDRESS);
    Wire.write(reg);
    Wire.endTransmission(false);

    Wire.requestFrom(HMC5883L_ADDRESS,1,true);

    return Wire.read();
}

// ////====================================================////
// ////               CONFIGURATION FUNCTIONS              ////
// ////====================================================////

void HMC5883L_Set_Averaging(Averaging_Mode averaging)
{
    uint8_t reg;
    reg = HMC5883L_Read_Register(HMC5883L_CRA);
    switch(averaging)
    {
        case AVERAGING_1:
            reg &= ~((1 << 6) | (1 << 5));
            break;
        case AVERAGING_2:
            reg &= ~(1 << 6);
            reg |=  (1 << 5);
            break;
        case AVERAGING_4:
            reg &= ~(1 << 5);
            reg |=  (1 << 6);
            break;
        case AVERAGING_8:
            reg |= (1 << 6) | (1 << 5);
            break;
        default:
            reg &= ~((1 << 6) | (1 << 5));
            break;
    }
    HMC5883L_Write_Register(HMC5883L_CRA, reg);
}

void HMC5883L_Set_Output_Rate(Output_Rate rate)
{
    uint8_t reg;
    reg = HMC5883L_Read_Register(HMC5883L_CRA);
    switch(rate)
    {
        case RATE_0_75:
            reg &= ~((1 << 4) | (1 << 3) | (1 << 2));
            break;
        case RATE_1_5:
            reg &= ~((1 << 4) | (1 << 3));
            reg |=  (1 << 2);
            break;
        case RATE_3:
            reg &= ~((1 << 4) | (1 << 2));
            reg |=  (1 << 3);
            break;
        case RATE_7_5:
            reg &= ~(1 << 4);
            reg |=  (1 << 3) | (1 << 2);
            break;
        case RATE_15:
            reg &= ~((1 << 3) | (1 << 2));
            reg |=  (1 << 4);
            break;
        case RATE_30:
            reg &= ~(1 << 3);
            reg |=  (1 << 4) | (1 << 2);
            break;
        case RATE_75:
            reg &= ~(1 << 2);
            reg |=  (1 << 4) | (1 << 3);
            break;
        default:
            reg &= ~((1 << 4) | (1 << 3) | (1 << 2)); // RATE 15
            break;
    }
    HMC5883L_Write_Register(HMC5883L_CRA, reg);
}

void HMC5883L_Set_Bias_Mode(Bias_Mode bias)
{
    uint8_t reg;
    reg = HMC5883L_Read_Register(HMC5883L_CRA);
    switch(bias)
    {
        case BIAS_NORMAL:
            reg &= ~((1 << 1) | (1 << 0));
            break;
        case BIAS_POSITIVE:
            reg &= ~(1 << 1);
            reg |=  (1 << 0);
            break;
        case BIAS_NEGATIVE:
            reg &= ~(1 << 0);
            reg |=  (1 << 1);
            break;
        default:
            reg &= ~((1 << 1) | (1 << 0));
            break;
    }
    HMC5883L_Write_Register(HMC5883L_CRA, reg);
}

void HMC5883L_Set_Gain(Gain_Mode gain)
{
    uint8_t reg;
    reg = HMC5883L_Read_Register(HMC5883L_CRB);
    switch(gain)
    {
        case FIELD_RANGE_0_88:
            reg &= ~((1 << 7) | (1 << 6) | (1 << 5));
            Magnetic_Sensitivity = 1370.0;
            break;
        case FIELD_RANGE_1_3:
            reg &= ~((1 << 7) | (1 << 6));
            reg |=  (1 << 5);
            Magnetic_Sensitivity = 1090.0;
            break;
        case FIELD_RANGE_1_9:
            reg &= ~((1 << 7) | (1 << 5));
            reg |=  (1 << 6);
            Magnetic_Sensitivity = 820.0;
            break;
        case FIELD_RANGE_2_5:
            reg &= ~(1 << 7);
            reg |=  (1 << 6) | (1 << 5);
            Magnetic_Sensitivity = 660.0;
            break;
        case FIELD_RANGE_4_0:
            reg &= ~(1 << 6);
            reg |=  (1 << 7) | (1 << 5);
            Magnetic_Sensitivity = 440.0;
            break;
        case FIELD_RANGE_4_7:
            reg &= ~(1 << 5);
            reg |=  (1 << 7) | (1 << 6);
            Magnetic_Sensitivity = 390.0;
            break;
        case FIELD_RANGE_5_6:
            reg |= (1 << 7) | (1 << 6);
            reg &= ~(1 << 5);
            Magnetic_Sensitivity = 330.0;
            break;
        case FIELD_RANGE_8_1:
            reg |= (1 << 7) | (1 << 6) | (1 << 5);
            Magnetic_Sensitivity = 230.0;
            break;
        default:
            reg &= ~((1 << 7) | (1 << 6));
            reg |=  (1 << 5);
            Magnetic_Sensitivity = 1090.0;
            break;
    }
    HMC5883L_Write_Register(HMC5883L_CRB, reg);
}

void HMC5883L_Set_Measurement_Mode(Measurement_Mode mode)
{
    uint8_t reg;
    reg = HMC5883L_Read_Register(HMC5883L_MODE);
    switch(mode)
    {
        case CONTINUOUS_MODE:
            reg &= ~((1 << 1) | (1 << 0));
            break;
        case SINGLE_MODE:
            reg &= ~(1 << 1);
            reg |=  (1 << 0);
            break;
        case IDLE_MODE:
            reg &= ~(1 << 0);
            reg |=  (1 << 1);
            break;
        default:
            reg &= ~((1 << 1) | (1 << 0));
            break;
    }
    HMC5883L_Write_Register(HMC5883L_MODE, reg);
}

// ////====================================================////
// ////                  DATA FUNCTIONS                    ////
// ////====================================================////

void HMC5883L_Read_Raw()
{
    Wire.beginTransmission(HMC5883L_ADDRESS);
    Wire.write(HMC5883L_DATA_X_MSB);
    Wire.endTransmission(false);

    Wire.requestFrom(HMC5883L_ADDRESS,6,true);

    MagX = (Wire.read() << 8) | Wire.read();
    MagZ = (Wire.read() << 8) | Wire.read();
    MagY = (Wire.read() << 8) | Wire.read();
}




// ////====================================================////
// ////                      MAIN                          ////
// ////====================================================////

#include "MPU6050_clone.h"

MPU6050_clone mpu;

void print()
{
    // Serial.print("Gyro: ");
    // Serial.print(mpu.getGyroX()); Serial.print(", ");
    // Serial.print(mpu.getGyroY()); Serial.print(", ");
    // Serial.print(mpu.getGyroZ());

    // Serial.print(" | Acc: ");
    // Serial.print(mpu.getAccX()); Serial.print(", ");
    // Serial.print(mpu.getAccY()); Serial.print(", ");
    // Serial.print(mpu.getAccZ());

    // Serial.print(" | Temp: ");
    // Serial.print(mpu.getTemp());

    Serial.print(" | Mag: ");
    Serial.print(MagX); Serial.print(", ");
    Serial.print(MagY); Serial.print(", ");
    Serial.print(MagZ);

    Serial.println();
}


void setup()
{
    Serial.begin(115200);
    while(!Serial);
    delay(2000);
    Wire.begin(10,9);
    Wire.setClock(400000);
    mpu.begin(10, 9);
    mpu.enableBypass();
    //mpu.setAccRange(MPU6050_clone::AFS_SEL_4G);
    //mpu.setGyroRange(MPU6050_clone::FS_SEL_2000);
    HMC5883L_Set_Averaging(AVERAGING_8);
    HMC5883L_Set_Output_Rate(RATE_75);
    HMC5883L_Set_Bias_Mode(BIAS_NORMAL);
    HMC5883L_Set_Gain(FIELD_RANGE_1_3);
    HMC5883L_Set_Measurement_Mode(CONTINUOUS_MODE);
    delay(100);

}


void loop()
{
  mpu.readGAT();
   HMC5883L_Read_Raw();
   print();
}