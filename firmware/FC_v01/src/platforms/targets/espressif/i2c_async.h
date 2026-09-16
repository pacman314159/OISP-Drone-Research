#pragma once
#include "platforms/hal/hal_i2c_async.h"
#include <driver/i2c.h>

class ESP32_I2C_ASYNC : public HAL_I2C_ASYNC {
public:
  ESP32_I2C_ASYNC(i2c_port_t port, int sda_pin, int scl_pin);
  ~ESP32_I2C_ASYNC() override;

  bool init(uint32_t frequency_hz = 400000) override;

  bool write_reg_async(uint8_t dev_addr, uint8_t reg_addr, uint8_t data, i2c_async_cb_t cb = nullptr, void* user_arg = nullptr) override;
  bool write_reg_multi_async(uint8_t dev_addr, uint8_t reg_addr, uint8_t* buf, size_t len, i2c_async_cb_t cb = nullptr, void* user_arg = nullptr) override;

  bool read_reg_async(uint8_t dev_addr, uint8_t reg_addr, uint8_t* out_data, i2c_async_cb_t cb = nullptr, void* user_arg = nullptr) override;
  bool read_reg_multi_async(uint8_t dev_addr, uint8_t reg_addr, uint8_t* buf, size_t len, i2c_async_cb_t cb = nullptr, void* user_arg = nullptr) override;

private:
  i2c_port_t _port;
  int _sda_pin;
  int _scl_pin;
  uint32_t _frequency_hz;
  bool _initialized;
};
