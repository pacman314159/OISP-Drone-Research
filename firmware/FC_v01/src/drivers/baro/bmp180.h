#pragma once
#include <cstdint>
#include <cmath>
#include "config.h"

// Conditional HAL abstraction selector (Sync when SYSTEM_I2C0_MODE_ASYNC is false)
#if (!SYSTEM_I2C0_MODE_ASYNC)
  #include "platforms/hal/hal_i2c.h"
  using HAL_I2C_BUS = HAL_I2C;
#else
  #include "platforms/hal/hal_i2c_async.h"
  using HAL_I2C_BUS = HAL_I2C_ASYNC;
#endif

constexpr uint8_t BMP180_I2C_ADDR = 0x77;
constexpr uint8_t BMP180_CHIP_ID  = 0x55;

enum Bmp180Register : uint8_t {
  REG_CALIB_START = 0xAA,
  REG_CHIP_ID     = 0xD0,
  REG_SOFT_RESET  = 0xE0,
  REG_CTRL_MEAS   = 0xF4,
  REG_OUT_MSB     = 0xF6,
  REG_OUT_LSB     = 0xF7,
  REG_OUT_XLSB    = 0xF8
};

enum Bmp180Cmd : uint8_t {
  CMD_READ_TEMP  = 0x2E,
  CMD_READ_PRESS = 0x34,
  CMD_SOFT_RESET = 0xB6
};

enum BmpOss : uint8_t {
  BMP_OSS_ULTRA_LOW_POWER = 0, // 1 sample,   max 4.5 ms
  BMP_OSS_STANDARD        = 1, // 2 samples,  max 7.5 ms
  BMP_OSS_HIGH_RES        = 2, // 4 samples,  max 13.5 ms
  BMP_OSS_ULTRA_HIGH_RES  = 3  // 8 samples,  max 25.5 ms
};

struct Bmp180Calib {
  int16_t  ac1;
  int16_t  ac2;
  int16_t  ac3;
  uint16_t ac4;
  uint16_t ac5;
  uint16_t ac6;
  int16_t  b1;
  int16_t  b2;
  int16_t  mb;
  int16_t  mc;
  int16_t  md;
};

class BMP180 {
public:
  explicit BMP180(HAL_I2C_BUS& i2c_bus, uint8_t dev_addr = BMP180_I2C_ADDR);

  bool init();

  // Non-blocking 2-phase measurement methods
  bool start_temperature_meas();
  bool read_temperature_raw(int32_t& ut);

  bool start_pressure_meas(BmpOss oss);
  bool read_pressure_raw(int32_t& up);

  // Pure computation routines
  int32_t calculate_b5(int32_t ut);
  float calculate_temperature(int32_t ut);
  float calculate_pressure(int32_t up, BmpOss oss);

  // Getters & configuration
  int32_t get_cached_b5() const{ return _b5; }
  void set_oversampling(BmpOss oss){ _oss = oss; }
  BmpOss get_oversampling() const{ return _oss; }
  const Bmp180Calib& get_calib() const{ return _calib; }

  // Timing & conversion helpers
  static uint32_t get_conversion_delay_ms(BmpOss oss);
  static uint32_t get_temp_conversion_delay_ms(){ return 5; }
  static float pressure_to_altitude(float pressure_pa, float sea_level_pa = 101325.0f);

private:
  HAL_I2C_BUS& _i2c;
  uint8_t      _dev_addr;
  BmpOss       _oss;
  Bmp180Calib  _calib;
  int32_t      _b5;

  bool read_calibration_data();
};
