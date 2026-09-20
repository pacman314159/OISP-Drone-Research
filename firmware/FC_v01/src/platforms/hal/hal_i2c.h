#pragma once
#include <cstdint>
#include <cstddef>
#include "config.h"

class HAL_I2C {
public:
  virtual ~HAL_I2C() = default;
  virtual bool init(uint32_t frequency_hz = 400000) = 0;
  virtual bool write_reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t data) = 0;
  virtual bool write_reg_multi(uint8_t dev_addr, uint8_t reg_addr, uint8_t* buf, size_t len) = 0;
  virtual bool read_reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t* out_data) = 0;
  virtual bool read_reg_multi(uint8_t dev_addr, uint8_t reg_addr, uint8_t* buf, size_t len) = 0;
};

// Global Target Bus Accessors (Pure HAL interface, defined per platform target build)
HAL_I2C& get_i2c0_bus();

#if (!defined(DISABLE_I2C1_FOR_JTAG) || (DISABLE_I2C1_FOR_JTAG == 0))
HAL_I2C& get_i2c1_bus();
#endif

