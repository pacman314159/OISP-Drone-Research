#include "platforms/targets/espressif/i2c_async.h"
#include <esp_err.h>
#include <esp_log.h>
#include <cstring>

static const char* TAG = "ESP32_I2C_ASYNC";

ESP32_I2C_ASYNC::ESP32_I2C_ASYNC(i2c_port_t port, int sda_pin, int scl_pin)
  : _port(port), _sda_pin(sda_pin), _scl_pin(scl_pin), _frequency_hz(400000), _initialized(false) {}

ESP32_I2C_ASYNC::~ESP32_I2C_ASYNC(){
  if(_initialized){
    i2c_driver_delete(_port);
    _initialized = false;
  }
}

bool ESP32_I2C_ASYNC::init(uint32_t frequency_hz){
  _frequency_hz = frequency_hz;

  i2c_config_t conf = {};
  conf.mode = I2C_MODE_MASTER;
  conf.sda_io_num = (gpio_num_t)_sda_pin;
  conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
  conf.scl_io_num = (gpio_num_t)_scl_pin;
  conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
  conf.master.clk_speed = _frequency_hz;

  esp_err_t err = i2c_param_config(_port, &conf);
  if(err != ESP_OK){
    ESP_LOGE(TAG, "i2c_param_config failed: %s", esp_err_to_name(err));
    return false;
  }

  err = i2c_driver_install(_port, conf.mode, 0, 0, 0);
  if(err != ESP_OK){
    ESP_LOGE(TAG, "i2c_driver_install failed: %s", esp_err_to_name(err));
    return false;
  }

  _initialized = true;
  return true;
}

bool ESP32_I2C_ASYNC::write_reg_async(uint8_t dev_addr, uint8_t reg_addr, uint8_t data, i2c_async_cb_t cb, void* user_arg){
  return write_reg_multi_async(dev_addr, reg_addr, &data, 1, cb, user_arg);
}

bool ESP32_I2C_ASYNC::write_reg_multi_async(uint8_t dev_addr, uint8_t reg_addr, uint8_t* buf, size_t len, i2c_async_cb_t cb, void* user_arg){
  if(!_initialized) return false;

  uint8_t write_buf[16];
  write_buf[0] = reg_addr;
  if(len > 0 && len < 15)
    memcpy(&write_buf[1], buf, len);

  esp_err_t err = i2c_master_write_to_device(_port, dev_addr, write_buf, len + 1, pdMS_TO_TICKS(10));
  bool success = (err == ESP_OK);

  if(cb) cb(success, user_arg);
  return success;
}

bool ESP32_I2C_ASYNC::read_reg_async(uint8_t dev_addr, uint8_t reg_addr, uint8_t* out_data, i2c_async_cb_t cb, void* user_arg){
  return read_reg_multi_async(dev_addr, reg_addr, out_data, 1, cb, user_arg);
}

bool ESP32_I2C_ASYNC::read_reg_multi_async(uint8_t dev_addr, uint8_t reg_addr, uint8_t* buf, size_t len, i2c_async_cb_t cb, void* user_arg){
  if(!_initialized) return false;

  esp_err_t err = i2c_master_write_read_device(_port, dev_addr, &reg_addr, 1, buf, len, pdMS_TO_TICKS(10));
  bool success = (err == ESP_OK);

  if(cb) cb(success, user_arg);
  return success;
}
