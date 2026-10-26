#include "platforms/targets/espressif/i2c_async.h"
#include <esp_err.h>
#include <esp_log.h>
#include <cstring>

static const char* TAG = "ESP32_I2C_ASYNC";

ESP32_I2C_ASYNC::ESP32_I2C_ASYNC(int port, int sda_pin, int scl_pin)
  : _port(port), _sda_pin(sda_pin), _scl_pin(scl_pin), _frequency_hz(400000), _initialized(false), _bus_handle(nullptr){}

ESP32_I2C_ASYNC::~ESP32_I2C_ASYNC(){
  if(_initialized && _bus_handle != nullptr){
    i2c_del_master_bus(_bus_handle);
    _bus_handle = nullptr;
    _initialized = false;
  }
}

bool ESP32_I2C_ASYNC::init(uint32_t frequency_hz){
  _frequency_hz = frequency_hz;

  i2c_master_bus_config_t bus_config = {};
  bus_config.i2c_port = static_cast<i2c_port_num_t>(_port);
  bus_config.sda_io_num = static_cast<gpio_num_t>(_sda_pin);
  bus_config.scl_io_num = static_cast<gpio_num_t>(_scl_pin);
  bus_config.clk_source = I2C_CLK_SRC_DEFAULT;
  bus_config.glitch_ignore_cnt = 7;
  bus_config.flags.enable_internal_pullup = 1;

  esp_err_t err = i2c_new_master_bus(&bus_config, &_bus_handle);
  if(err != ESP_OK){
    ESP_LOGE(TAG, "i2c_new_master_bus failed: %s", esp_err_to_name(err));
    return false;
  }

  _initialized = true;
  return true;
}

bool ESP32_I2C_ASYNC::write_reg_async(uint8_t dev_addr, uint8_t reg_addr, uint8_t data, i2c_async_cb_t cb, void* user_arg){
  return write_reg_multi_async(dev_addr, reg_addr, &data, 1, cb, user_arg);
}

bool ESP32_I2C_ASYNC::write_reg_multi_async(uint8_t dev_addr, uint8_t reg_addr, uint8_t* buf, size_t len, i2c_async_cb_t cb, void* user_arg){
  if(!_initialized || _bus_handle == nullptr) return false;

  i2c_device_config_t dev_cfg = {};
  dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
  dev_cfg.device_address = dev_addr;
  dev_cfg.scl_speed_hz = _frequency_hz;

  i2c_master_dev_handle_t dev_handle = nullptr;
  if(i2c_master_bus_add_device(_bus_handle, &dev_cfg, &dev_handle) != ESP_OK) return false;

  uint8_t write_buf[16];
  write_buf[0] = reg_addr;
  if(len > 0 && len < 15)
    memcpy(&write_buf[1], buf, len);

  esp_err_t err = i2c_master_transmit(dev_handle, write_buf, len + 1, 10);
  i2c_master_bus_rm_device(dev_handle);

  bool success = (err == ESP_OK);
  if(cb) cb(success, user_arg);
  return success;
}

bool ESP32_I2C_ASYNC::read_reg_async(uint8_t dev_addr, uint8_t reg_addr, uint8_t* out_data, i2c_async_cb_t cb, void* user_arg){
  return read_reg_multi_async(dev_addr, reg_addr, out_data, 1, cb, user_arg);
}

bool ESP32_I2C_ASYNC::read_reg_multi_async(uint8_t dev_addr, uint8_t reg_addr, uint8_t* buf, size_t len, i2c_async_cb_t cb, void* user_arg){
  if(!_initialized || _bus_handle == nullptr) return false;

  i2c_device_config_t dev_cfg = {};
  dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
  dev_cfg.device_address = dev_addr;
  dev_cfg.scl_speed_hz = _frequency_hz;

  i2c_master_dev_handle_t dev_handle = nullptr;
  if(i2c_master_bus_add_device(_bus_handle, &dev_cfg, &dev_handle) != ESP_OK) return false;

  esp_err_t err = i2c_master_transmit_receive(dev_handle, &reg_addr, 1, buf, len, 10);
  i2c_master_bus_rm_device(dev_handle);

  bool success = (err == ESP_OK);
  if(cb) cb(success, user_arg);
  return success;
}
