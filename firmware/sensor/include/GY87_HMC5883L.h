#ifndef GY87_HMC5883L_H
#define GY87_HMC5883L_H

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

#define DEFAULT_GAIN_LSB_PER_GAUSS 1090.0


enum AvgMode
{
    AVERAGING_1,
    AVERAGING_2,
    AVERAGING_4,
    AVERAGING_8
};

enum OutputRate
{
    RATE_0_75,
    RATE_1_5,
    RATE_3,
    RATE_7_5,
    RATE_15,
    RATE_30,
    RATE_75
};

enum BiasMode
{
    BIAS_NORMAL,
    BIAS_POSITIVE,
    BIAS_NEGATIVE
};

enum GainMode
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

enum MeasMode
{
    CONTINUOUS_MODE,
    SINGLE_MODE,
    IDLE_MODE
};

class GY87_HMC5883L{
private:
  float sensitivity;
  int16_t xRaw, yRaw, zRaw;
  float x, y, z;

public:
  GY87_HMC5883L();

  void setAveraging(AvgMode avg);
  void setOutputRate(OutputRate rate);
  void setBiasMode(BiasMode bias);
  void setGain(GainMode gain);
  void setMeasMode(MeasMode meas);

  void getRawAll();
  int16_t getRawX();
  int16_t getRawY();
  int16_t getRawZ();

  void getAllData(float &_x, float &_y, float &_z);
  float getX();
  float getY();
  float getZ();

private:
  void writeRegister(uint8_t reg, uint8_t data);
  uint8_t readRegister(uint8_t reg);
};

#endif
