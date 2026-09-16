#include "platforms/targets/espressif/i2c.h"

ESP32_I2C::ESP32_I2C(TwoWire& wire_instance, int sda_pin, int scl_pin)
  : _wire(wire_instance), _sda_pin(sda_pin), _scl_pin(scl_pin){}

bool ESP32_I2C::init(uint32_t frequency_hz){
  bool ok = _wire.begin(_sda_pin, _scl_pin, frequency_hz);
  _wire.setTimeOut(10); // 10ms timeout prevents flight loop lockup on sensor bus failure
  return ok;
}

bool ESP32_I2C::write_reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t data){
  return write_reg_multi(dev_addr, reg_addr, &data, 1);
}

bool ESP32_I2C::write_reg_multi(uint8_t dev_addr, uint8_t reg_addr, uint8_t* buf, size_t len){
  _wire.beginTransmission(dev_addr);
  _wire.write(reg_addr);
  for(size_t i = 0; i < len; i++)
    _wire.write(buf[i]);
  return (_wire.endTransmission() == 0);
}

bool ESP32_I2C::read_reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t* out_data){
  return read_reg_multi(dev_addr, reg_addr, out_data, 1);
}

bool ESP32_I2C::read_reg_multi(uint8_t dev_addr, uint8_t reg_addr, uint8_t* buf, size_t len){
  _wire.beginTransmission(dev_addr);
  _wire.write(reg_addr);
  if(_wire.endTransmission(false) != 0)
    return false;

  size_t bytes_read = _wire.requestFrom(dev_addr, len, true);
  if(bytes_read != len) return false;

  for(size_t i = 0; i < len; i++)
    buf[i] = _wire.read();

  return true;
}

#include "config.h"

HAL_I2C& get_i2c0_bus(){
  static ESP32_I2C i2c0_inst(Wire, I2C0_SDA_PIN, I2C0_SCL_PIN);
  return i2c0_inst;
}

#if (!defined(DISABLE_I2C1_FOR_JTAG) || (DISABLE_I2C1_FOR_JTAG == 0))
HAL_I2C& get_i2c1_bus(){
  static ESP32_I2C i2c1_inst(Wire1, I2C1_SDA_PIN, I2C1_SCL_PIN);
  return i2c1_inst;
}
#endif
