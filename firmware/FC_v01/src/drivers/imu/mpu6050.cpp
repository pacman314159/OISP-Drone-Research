#include "drivers/imu/mpu6050.h"

MPU6050::MPU6050(HAL_I2C_BUS& i2c_bus, uint8_t dev_addr)
  : _i2c(i2c_bus),
    _dev_addr(dev_addr),
    _accel_sens(DEFAULT_ACCEL_SENS),
    _gyro_sens(DEFAULT_GYRO_SENS),
    _accel_raw_x(0),
    _accel_raw_y(0),
    _accel_raw_z(0),
    _gyro_raw_x(0),
    _gyro_raw_y(0),
    _gyro_raw_z(0),
    _temp_raw(0){}

bool MPU6050::init(){
  uint8_t who_am_i = 0;
  if(!_i2c.read_reg(_dev_addr, REG_WHO_AM_I, &who_am_i) || who_am_i != 0x68)
    return false; // MPU6050 not responding or invalid ID

  if(!enable_sleep(false))
    return false;

  if(!set_clock_source(CLOCK_PLL_XGYRO))
    return false;

  if(!set_gyro_range(GYRO_RANGE_250DPS))
    return false;

  if(!set_accel_range(ACCEL_RANGE_2G))
    return false;

  return true;
}

bool MPU6050::enable_sleep(bool enable){
  uint8_t reg = 0;
  if(!_i2c.read_reg(_dev_addr, REG_PWR_MGMT_1, &reg))
    return false;

  if(enable)
    reg |= (1 << 6);
  else
    reg &= ~(1 << 6);

  return _i2c.write_reg(_dev_addr, REG_PWR_MGMT_1, reg);
}

bool MPU6050::set_clock_source(ClockSource source){
  uint8_t reg = 0;
  if(!_i2c.read_reg(_dev_addr, REG_PWR_MGMT_1, &reg))
    return false;

  reg &= ~0x07; // Clear bits 0..2
  reg |= (static_cast<uint8_t>(source) & 0x07);
  return _i2c.write_reg(_dev_addr, REG_PWR_MGMT_1, reg);
}

bool MPU6050::set_gyro_range(GyroRange range){
  uint8_t reg = 0;
  if(!_i2c.read_reg(_dev_addr, REG_GYRO_CFG, &reg))
    return false;

  reg &= ~(0x18); // Clear bits 3 and 4
  reg |= (static_cast<uint8_t>(range) << 3);

  switch(range){
    case GYRO_RANGE_250DPS:  _gyro_sens = 131.0f; break;
    case GYRO_RANGE_500DPS:  _gyro_sens = 65.5f;  break;
    case GYRO_RANGE_1000DPS: _gyro_sens = 32.8f;  break;
    case GYRO_RANGE_2000DPS: _gyro_sens = 16.4f;  break;
  }

  return _i2c.write_reg(_dev_addr, REG_GYRO_CFG, reg);
}

bool MPU6050::set_accel_range(AccelRange range){
  uint8_t reg = 0;
  if(!_i2c.read_reg(_dev_addr, REG_ACCEL_CFG, &reg))
    return false;

  reg &= ~(0x18); // Clear bits 3 and 4
  reg |= (static_cast<uint8_t>(range) << 3);

  switch(range){
    case ACCEL_RANGE_2G:  _accel_sens = 16384.0f; break;
    case ACCEL_RANGE_4G:  _accel_sens = 8192.0f;  break;
    case ACCEL_RANGE_8G:  _accel_sens = 4096.0f;  break;
    case ACCEL_RANGE_16G: _accel_sens = 2048.0f;  break;
  }

  return _i2c.write_reg(_dev_addr, REG_ACCEL_CFG, reg);
}

bool MPU6050::enable_bypass(){
  uint8_t user_ctrl = 0;
  if(!_i2c.read_reg(_dev_addr, REG_USER_CTRL, &user_ctrl))
    return false;

  user_ctrl &= ~(1 << 5); // Disable I2C Master mode
  if(!_i2c.write_reg(_dev_addr, REG_USER_CTRL, user_ctrl))
    return false;

  uint8_t int_cfg = 0;
  if(!_i2c.read_reg(_dev_addr, REG_INT_CFG, &int_cfg))
    return false;

  int_cfg |= (1 << 1); // Enable I2C Bypass
  return _i2c.write_reg(_dev_addr, REG_INT_CFG, int_cfg);
}

bool MPU6050::get_all_data(IMUSample& sample){
  uint8_t raw_buf[14];
  if(!_i2c.read_reg_multi(_dev_addr, REG_ACCEL_X, raw_buf, 14))
    return false;

  _accel_raw_x = static_cast<int16_t>((raw_buf[0] << 8) | raw_buf[1]);
  _accel_raw_y = static_cast<int16_t>((raw_buf[2] << 8) | raw_buf[3]);
  _accel_raw_z = static_cast<int16_t>((raw_buf[4] << 8) | raw_buf[5]);
  _temp_raw    = static_cast<int16_t>((raw_buf[6] << 8) | raw_buf[7]);
  _gyro_raw_x  = static_cast<int16_t>((raw_buf[8] << 8) | raw_buf[9]);
  _gyro_raw_y  = static_cast<int16_t>((raw_buf[10] << 8) | raw_buf[11]);
  _gyro_raw_z  = static_cast<int16_t>((raw_buf[12] << 8) | raw_buf[13]);

  // Physical unit conversions (m/s^2, rad/s, Celsius)
  sample.a[0] = (_accel_raw_x / _accel_sens) * GRAVITY_MSS;
  sample.a[1] = (_accel_raw_y / _accel_sens) * GRAVITY_MSS;
  sample.a[2] = (_accel_raw_z / _accel_sens) * GRAVITY_MSS;

  sample.g[0] = (_gyro_raw_x / _gyro_sens) * DEG_TO_RAD;
  sample.g[1] = (_gyro_raw_y / _gyro_sens) * DEG_TO_RAD;
  sample.g[2] = (_gyro_raw_z / _gyro_sens) * DEG_TO_RAD;

  sample.t = (_temp_raw / 340.0f) + 36.53f;

  return true;
}

#if (SYSTEM_I2C0_MODE_ASYNC)
bool MPU6050::get_all_data_async(i2c_async_cb_t cb, void* user_arg){
  return false;
}
#endif
