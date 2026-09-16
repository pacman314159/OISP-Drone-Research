#pragma once
#include "platforms/hal/hal_i2c.h"
#include <Wire.h>

class ESP32_I2C : public HAL_I2C {
public:
  ESP32_I2C(TwoWire& wire_instance, int sda_pin, int scl_pin);

  bool init(uint32_t frequency_hz = 400000) override;
  bool write_reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t data) override;
  bool write_reg_multi(uint8_t dev_addr, uint8_t reg_addr, uint8_t* buf, size_t len) override;
  bool read_reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t* out_data) override;
  bool read_reg_multi(uint8_t dev_addr, uint8_t reg_addr, uint8_t* buf, size_t len) override;

private:
  TwoWire& _wire;
  int _sda_pin;
  int _scl_pin;
};
