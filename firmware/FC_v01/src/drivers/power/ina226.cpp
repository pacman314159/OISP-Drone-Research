#include "drivers/power/ina226.h"
#include <cmath>

INA226::INA226(HAL_I2C_BUS& i2c_bus, uint8_t dev_addr)
  : _i2c(i2c_bus),
    _dev_addr(dev_addr),
    _shunt_resistor_ohm(INA226_SHUNT_RESISTOR_OHM),
    _current_lsb_ma(INA226_CURRENT_LSB_MA),
    _power_lsb_mw(INA226_POWER_LSB_MW),
    _initialized(false),
    _raw_bus_voltage(0),
    _raw_shunt_voltage(0),
    _raw_current(0),
    _raw_power(0){}

bool INA226::write_register(uint8_t reg_addr, uint16_t value){
  uint8_t buf[2];
  buf[0] = static_cast<uint8_t>((value >> 8) & 0xFF);
  buf[1] = static_cast<uint8_t>(value & 0xFF);
  return _i2c.write_reg_multi(_dev_addr, reg_addr, buf, 2);
}

bool INA226::read_register(uint8_t reg_addr, uint16_t& out_value){
  uint8_t buf[2] = {0, 0};
  if(!_i2c.read_reg_multi(_dev_addr, reg_addr, buf, 2)) return false;
  out_value = (static_cast<uint16_t>(buf[0]) << 8) | buf[1];
  return true;
}

bool INA226::verify_chip_id(){
  uint16_t manuf_id = 0;
  uint16_t die_id   = 0;
  if(!read_register(INA_REG_MANUF_ID, manuf_id)) return false;
  if(!read_register(INA_REG_DIE_ID, die_id)) return false;

  return (manuf_id == INA226_MANUFACTURER_ID && (die_id & 0xFFF0) == (INA226_DIE_ID_VAL & 0xFFF0));
}

bool INA226::calibrate(uint16_t cal_val){
  return write_register(INA_REG_CALIBRATION, cal_val);
}

bool INA226::set_averaging(InaAvgMode avg){
  uint16_t reg = 0;
  if(!read_register(INA_REG_CONFIG, reg)) return false;
  reg &= ~(0x0E00); // Clear bits 11:9
  reg |= ((static_cast<uint16_t>(avg) & 0x07) << 9);
  return write_register(INA_REG_CONFIG, reg);
}

bool INA226::set_bus_conversion_time(InaConvTime conv_time){
  uint16_t reg = 0;
  if(!read_register(INA_REG_CONFIG, reg)) return false;
  reg &= ~(0x01C0); // Clear bits 8:6
  reg |= ((static_cast<uint16_t>(conv_time) & 0x07) << 6);
  return write_register(INA_REG_CONFIG, reg);
}

bool INA226::set_shunt_conversion_time(InaConvTime conv_time){
  uint16_t reg = 0;
  if(!read_register(INA_REG_CONFIG, reg)) return false;
  reg &= ~(0x0038); // Clear bits 5:3
  reg |= ((static_cast<uint16_t>(conv_time) & 0x07) << 3);
  return write_register(INA_REG_CONFIG, reg);
}

bool INA226::set_mode(InaMode mode){
  uint16_t reg = 0;
  if(!read_register(INA_REG_CONFIG, reg)) return false;
  reg &= ~(0x0007); // Clear bits 2:0
  reg |= (static_cast<uint16_t>(mode) & 0x07);
  return write_register(INA_REG_CONFIG, reg);
}

bool INA226::init(float shunt_resistor_ohm, float current_lsb_ma){
  _shunt_resistor_ohm = shunt_resistor_ohm;
  _current_lsb_ma     = current_lsb_ma;
  _power_lsb_mw       = 25.0f * _current_lsb_ma; // Power LSB = 25 * Current LSB

  _i2c.init(I2C_FREQ_HZ);

  if(!_i2c.probe(_dev_addr)) return false;

  if(!verify_chip_id()) return false;

  // Base configuration word 0x4000 (Bits 14:12 reserved as 100 per datasheet)
  if(!write_register(INA_REG_CONFIG, 0x4000)) return false;

  // Modular configuration setters (matching MPU6050 & HMC5883L init pattern)
  if(!set_averaging(INA_AVG_4)) return false;
  if(!set_bus_conversion_time(INA_CONV_1100US)) return false;
  if(!set_shunt_conversion_time(INA_CONV_1100US)) return false;
  if(!set_mode(INA_MODE_SHUNT_BUS_CONT)) return false;

  // CAL = trunc(0.00512 / (Current_LSB_A * Rshunt))
  float current_lsb_a = _current_lsb_ma / 1000.0f;
  float cal_calc = 0.00512f / (current_lsb_a * _shunt_resistor_ohm);
  uint16_t cal_val = static_cast<uint16_t>(cal_calc);
  if(cal_val == 0) cal_val = INA226_CALIBRATION_VAL;

  if(!calibrate(cal_val)) return false;

  _initialized = true;
  return true;
}

bool INA226::get_all_data_raw(uint16_t& raw_bus_v, int16_t& raw_shunt_v, int16_t& raw_current, uint16_t& raw_power){
  if(!_initialized) return false;

  uint16_t raw_s = 0;
  uint16_t raw_i = 0;

  if(!read_register(INA_REG_BUS_V, _raw_bus_voltage) ||
     !read_register(INA_REG_SHUNT_V, raw_s) ||
     !read_register(INA_REG_CURRENT, raw_i) ||
     !read_register(INA_REG_POWER, _raw_power)){
    _initialized = false;
    return false;
  }

  _raw_shunt_voltage = static_cast<int16_t>(raw_s);
  _raw_current       = static_cast<int16_t>(raw_i);

  raw_bus_v   = _raw_bus_voltage;
  raw_shunt_v = _raw_shunt_voltage;
  raw_current = _raw_current;
  raw_power   = _raw_power;

  return true;
}

bool INA226::get_all_data(PowerSample& sample){
  uint16_t raw_v = 0;
  int16_t  raw_s = 0;
  int16_t  raw_i = 0;
  uint16_t raw_p = 0;

  if(!get_all_data_raw(raw_v, raw_s, raw_i, raw_p)) return false;

  sample.voltage_mv = static_cast<float>(raw_v) * INA226_VOLTAGE_LSB_MV;
  sample.shunt_uv   = static_cast<float>(raw_s) * 2.5f;
  sample.current_ma = static_cast<float>(raw_i) * _current_lsb_ma;
  sample.power_mw   = static_cast<float>(raw_p) * _power_lsb_mw;

  return true;
}

bool INA226::read_bus_voltage_mv(float& out_voltage_mv){
  uint16_t raw_v = 0;
  if(!read_register(INA_REG_BUS_V, raw_v)) return false;
  _raw_bus_voltage = raw_v;
  out_voltage_mv = static_cast<float>(raw_v) * INA226_VOLTAGE_LSB_MV;
  return true;
}

bool INA226::read_shunt_voltage_uv(float& out_shunt_uv){
  uint16_t raw_s = 0;
  if(!read_register(INA_REG_SHUNT_V, raw_s)) return false;
  _raw_shunt_voltage = static_cast<int16_t>(raw_s);
  out_shunt_uv = static_cast<float>(_raw_shunt_voltage) * 2.5f; // 2.5 uV per LSB
  return true;
}

bool INA226::read_current_ma(float& out_current_ma){
  uint16_t raw_i = 0;
  if(!read_register(INA_REG_CURRENT, raw_i)) return false;
  _raw_current = static_cast<int16_t>(raw_i);
  out_current_ma = static_cast<float>(_raw_current) * _current_lsb_ma;
  return true;
}

bool INA226::read_power_mw(float& out_power_mw){
  uint16_t raw_p = 0;
  if(!read_register(INA_REG_POWER, raw_p)) return false;
  _raw_power = raw_p;
  out_power_mw = static_cast<float>(raw_p) * _power_lsb_mw;
  return true;
}
