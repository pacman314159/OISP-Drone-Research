#pragma once
#include <cstdint>
#include "config.h"
#include "src/core/math/matrix.h"

// Conditional HAL abstraction selector (Sync when SYSTEM_I2C0_MODE_ASYNC is false)
#if (!SYSTEM_I2C0_MODE_ASYNC)
  #include "platforms/hal/hal_i2c.h"
  using HAL_I2C_BUS = HAL_I2C;
#else
  #include "platforms/hal/hal_i2c_async.h"
  using HAL_I2C_BUS = HAL_I2C_ASYNC;
#endif

constexpr uint8_t HMC5883L_I2C_ADDR = 0x1E;

// HMC5883L Hardware Register Address Map
enum Hmc5883lRegister : uint8_t {
  REG_CRA        = 0x00,
  REG_CRB        = 0x01,
  REG_MODE       = 0x02,
  REG_DATA_X_MSB = 0x03,
  REG_DATA_X_LSB = 0x04,
  REG_DATA_Z_MSB = 0x05,
  REG_DATA_Z_LSB = 0x06,
  REG_DATA_Y_MSB = 0x07,
  REG_DATA_Y_LSB = 0x08,
  REG_STATUS     = 0x09,
  REG_ID_A       = 0x0A,
  REG_ID_B       = 0x0B,
  REG_ID_C       = 0x0C
};

constexpr float DEFAULT_MAG_SENS = 1090.0f; // ±1.3 Gauss range: 1090 LSB/Gauss

enum HmcAvgMode : uint8_t {
  HMC_AVG_1 = 0,
  HMC_AVG_2 = 1,
  HMC_AVG_4 = 2,
  HMC_AVG_8 = 3
};

enum HmcOutputRate : uint8_t {
  HMC_RATE_0_75 = 0,
  HMC_RATE_1_5  = 1,
  HMC_RATE_3    = 2,
  HMC_RATE_7_5  = 3,
  HMC_RATE_15   = 4,
  HMC_RATE_30   = 5,
  HMC_RATE_75   = 6
};

enum HmcBiasMode : uint8_t {
  HMC_BIAS_NORMAL   = 0,
  HMC_BIAS_POSITIVE = 1,
  HMC_BIAS_NEGATIVE = 2
};

enum HmcGainMode : uint8_t {
  HMC_GAIN_0_88 = 0,
  HMC_GAIN_1_3  = 1,
  HMC_GAIN_1_9  = 2,
  HMC_GAIN_2_5  = 3,
  HMC_GAIN_4_0  = 4,
  HMC_GAIN_4_7  = 5,
  HMC_GAIN_5_6  = 6,
  HMC_GAIN_8_1  = 7
};

enum HmcMeasMode : uint8_t {
  HMC_MODE_CONTINUOUS = 0,
  HMC_MODE_SINGLE     = 1,
  HMC_MODE_IDLE       = 2
};

class HMC5883L {

public:
  explicit HMC5883L(HAL_I2C_BUS& i2c_bus, uint8_t dev_addr = HMC5883L_I2C_ADDR);

  bool init();
  bool set_averaging(HmcAvgMode avg);
  bool set_output_rate(HmcOutputRate rate);
  bool set_bias_mode(HmcBiasMode bias);
  bool set_gain(HmcGainMode gain);
  bool set_meas_mode(HmcMeasMode meas);

  // Synchronous multi-byte burst raw read & converted data
  bool get_all_data_raw(Vec3<int16_t>& mag_raw_msb);
  bool get_all_data(Vec3<float>& mag);

  // Raw data getters
  int16_t get_raw_mag_x(){ return _mag_raw_x; }
  int16_t get_raw_mag_y(){ return _mag_raw_y; }
  int16_t get_raw_mag_z(){ return _mag_raw_z; }

private:
  HAL_I2C_BUS& _i2c;
  uint8_t _dev_addr;

  float _mag_sens;
  float _inv_mag_sens;

  int16_t _mag_raw_x;
  int16_t _mag_raw_y;
  int16_t _mag_raw_z;
};
