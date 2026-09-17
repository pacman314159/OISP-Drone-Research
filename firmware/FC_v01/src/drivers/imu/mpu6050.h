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

constexpr uint8_t MPU6050_I2C_ADDR = 0x68;

// MPU6050 Hardware Register Address Map
enum Mpu6050Register : uint8_t {
  REG_SMPLRT_DIV = 0x19,
  REG_CONFIG     = 0x1A,
  REG_GYRO_CFG   = 0x1B,
  REG_ACCEL_CFG  = 0x1C,
  REG_INT_CFG    = 0x37,
  REG_ACCEL_X    = 0x3B,
  REG_USER_CTRL  = 0x6A,
  REG_PWR_MGMT_1 = 0x6B,
  REG_WHO_AM_I   = 0x75
};

constexpr float DEFAULT_ACCEL_SENS  = 16384.0f; // ±2g range: 16384 LSB/g
constexpr float DEFAULT_GYRO_SENS   = 131.0f;   // ±250 deg/s range: 131 LSB/(deg/s)
constexpr float GRAVITY_MSS         = 9.80665f; // Standard gravity in m/s^2
#ifndef DEG_TO_RAD
  constexpr float DEG_TO_RAD          = 0.017453292519943295f; // deg/s to rad/s conversion
#endif


enum GyroRange {
  GYRO_RANGE_250DPS  = 0,
  GYRO_RANGE_500DPS  = 1,
  GYRO_RANGE_1000DPS = 2,
  GYRO_RANGE_2000DPS = 3
};

enum AccelRange {
  ACCEL_RANGE_2G  = 0,
  ACCEL_RANGE_4G  = 1,
  ACCEL_RANGE_8G  = 2,
  ACCEL_RANGE_16G = 3
};

enum ClockSource {
  CLOCK_INTERNAL         = 0,
  CLOCK_PLL_XGYRO        = 1,
  CLOCK_PLL_YGYRO        = 2,
  CLOCK_PLL_ZGYRO        = 3,
  CLOCK_PLL_EXT32_768KHZ = 4,
  CLOCK_PLL_EXT19_2MHZ   = 5,
  CLOCK_KEEP_RESET       = 7
};

class MPU6050 {

public:
  explicit MPU6050(HAL_I2C_BUS& i2c_bus, uint8_t dev_addr = MPU6050_I2C_ADDR);

  bool init();
  bool enable_sleep(bool enable);
  bool enable_bypass();
  bool set_clock_source(ClockSource source);
  bool set_gyro_range(GyroRange range);
  bool set_accel_range(AccelRange range);

  // Synchronous multi-byte burst raw read & converted data
  bool get_all_data_raw(Vec3<int16_t>& accel_raw_msb, Vec3<int16_t>& gyro_raw_msb, int16_t& temp_raw_msb);
  bool get_all_data(Vec3<float>& accel, Vec3<float>& gyro, float& temp);

  // Raw data getters
  int16_t get_raw_accel_x(){ return _accel_raw_x; }
  int16_t get_raw_accel_y(){ return _accel_raw_y; }
  int16_t get_raw_accel_z(){ return _accel_raw_z; }
  int16_t get_raw_gyro_x(){ return _gyro_raw_x; }
  int16_t get_raw_gyro_y(){ return _gyro_raw_y; }
  int16_t get_raw_gyro_z(){ return _gyro_raw_z; }
  int16_t get_raw_temp(){ return _temp_raw; }

#if (SYSTEM_I2C0_MODE_ASYNC)
  bool get_all_data_async(i2c_async_cb_t cb, void* user_arg);
#endif

private:
  HAL_I2C_BUS& _i2c;
  uint8_t _dev_addr;

  float _accel_sens;
  float _gyro_sens;

  int16_t _accel_raw_x;
  int16_t _accel_raw_y;
  int16_t _accel_raw_z;
  int16_t _gyro_raw_x;
  int16_t _gyro_raw_y;
  int16_t _gyro_raw_z;
  int16_t _temp_raw;
};
