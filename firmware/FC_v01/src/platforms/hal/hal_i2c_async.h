#pragma once
#include <cstdint>
#include <cstddef>

// Asynchronous callback function pointer type
// success: true if transfer completed with ACK, false if NACK/timeout
// user_arg: pointer to user state or context passed when queuing request
typedef void (*i2c_async_cb_t)(bool success, void* user_arg);

class HAL_I2C_ASYNC {
public:
  virtual ~HAL_I2C_ASYNC() = default;

  virtual bool init(uint32_t frequency_hz = 400000) = 0;

  virtual bool write_reg_async(uint8_t dev_addr, uint8_t reg_addr, uint8_t data, i2c_async_cb_t cb = nullptr, void* user_arg = nullptr) = 0;
  virtual bool write_reg_multi_async(uint8_t dev_addr, uint8_t reg_addr, uint8_t* buf, size_t len, i2c_async_cb_t cb = nullptr, void* user_arg = nullptr) = 0;

  virtual bool read_reg_async(uint8_t dev_addr, uint8_t reg_addr, uint8_t* out_data, i2c_async_cb_t cb = nullptr, void* user_arg = nullptr) = 0;
  virtual bool read_reg_multi_async(uint8_t dev_addr, uint8_t reg_addr, uint8_t* buf, size_t len, i2c_async_cb_t cb = nullptr, void* user_arg = nullptr) = 0;
};
