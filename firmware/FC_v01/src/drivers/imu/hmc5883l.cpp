#include "drivers/imu/hmc5883l.h"

HMC5883L::HMC5883L(HAL_I2C_BUS& i2c_bus, uint8_t dev_addr)
  : _i2c(i2c_bus),
    _dev_addr(dev_addr),
    _mag_sens(DEFAULT_MAG_SENS),
    _inv_mag_sens(1.0f / DEFAULT_MAG_SENS),
    _mag_raw_x(0),
    _mag_raw_y(0),
    _mag_raw_z(0){}

bool HMC5883L::init(){
  uint8_t id_a = 0, id_b = 0, id_c = 0;
  if(!_i2c.read_reg(_dev_addr, REG_ID_A, &id_a) || id_a != 'H') return false;
  if(!_i2c.read_reg(_dev_addr, REG_ID_B, &id_b) || id_b != '4') return false;
  if(!_i2c.read_reg(_dev_addr, REG_ID_C, &id_c) || id_c != '3') return false;

  if(!set_averaging(HMC_AVG_1)) return false;
  if(!set_output_rate(HMC_RATE_75)) return false;
  if(!set_bias_mode(HMC_BIAS_NORMAL)) return false;
  if(!set_gain(HMC_GAIN_1_3)) return false;

  if(!set_meas_mode(HMC_MODE_SINGLE)) return false;

  return true;
}

bool HMC5883L::set_averaging(HmcAvgMode avg){
  uint8_t reg = 0;
  if(!_i2c.read_reg(_dev_addr, REG_CRA, &reg)) return false;

  reg &= ~(0x60);
  reg |= ((static_cast<uint8_t>(avg) & 0x03) << 5);

  return _i2c.write_reg(_dev_addr, REG_CRA, reg);
}

bool HMC5883L::set_output_rate(HmcOutputRate rate){
  uint8_t reg = 0;
  if(!_i2c.read_reg(_dev_addr, REG_CRA, &reg)) return false;

  reg &= ~(0x1C);
  reg |= ((static_cast<uint8_t>(rate) & 0x07) << 2);

  return _i2c.write_reg(_dev_addr, REG_CRA, reg);
}

bool HMC5883L::set_bias_mode(HmcBiasMode bias){
  uint8_t reg = 0;
  if(!_i2c.read_reg(_dev_addr, REG_CRA, &reg)) return false;

  reg &= ~(0x03);
  reg |= (static_cast<uint8_t>(bias) & 0x03);

  return _i2c.write_reg(_dev_addr, REG_CRA, reg);
}

bool HMC5883L::set_gain(HmcGainMode gain){
  // Per HMC5883L datasheet: CRB bits 4:0 must always be 0 for proper operation.
  // Writing a clean value (not read-modify-write) guarantees compliance.
  uint8_t reg = ((static_cast<uint8_t>(gain) & 0x07) << 5); // bits 4:0 guaranteed 0

  switch(gain){
    case HMC_GAIN_0_88: _mag_sens = 1370.0f; break;
    case HMC_GAIN_1_3:  _mag_sens = 1090.0f; break;
    case HMC_GAIN_1_9:  _mag_sens = 820.0f;  break;
    case HMC_GAIN_2_5:  _mag_sens = 660.0f;  break;
    case HMC_GAIN_4_0:  _mag_sens = 440.0f;  break;
    case HMC_GAIN_4_7:  _mag_sens = 390.0f;  break;
    case HMC_GAIN_5_6:  _mag_sens = 330.0f;  break;
    case HMC_GAIN_8_1:  _mag_sens = 230.0f;  break;
  }
  _inv_mag_sens = 1.0f / _mag_sens;

  return _i2c.write_reg(_dev_addr, REG_CRB, reg);
}

bool HMC5883L::set_meas_mode(HmcMeasMode meas){
  uint8_t reg = 0;
  if(!_i2c.read_reg(_dev_addr, REG_MODE, &reg)) return false;

  reg &= ~(0x03);
  reg |= (static_cast<uint8_t>(meas) & 0x03);

  return _i2c.write_reg(_dev_addr, REG_MODE, reg);
}

bool HMC5883L::get_all_data_raw(Vec3<int16_t>& mag_raw_msb){
  uint8_t raw_buf[6];
  if(!_i2c.read_reg_multi(_dev_addr, REG_DATA_X_MSB, raw_buf, 6)) return false;

  _mag_raw_x = static_cast<int16_t>((raw_buf[0] << 8) | raw_buf[1]);
  _mag_raw_z = static_cast<int16_t>((raw_buf[2] << 8) | raw_buf[3]);
  _mag_raw_y = static_cast<int16_t>((raw_buf[4] << 8) | raw_buf[5]);

  mag_raw_msb.x = _mag_raw_x;
  mag_raw_msb.y = _mag_raw_y;
  mag_raw_msb.z = _mag_raw_z;

  return true;
}

bool HMC5883L::get_all_data(Vec3<float>& mag){
  Vec3<int16_t> mag_raw_msb;
  if(!get_all_data_raw(mag_raw_msb)) return false;

  mag.x = static_cast<float>(mag_raw_msb.x);
  mag.y = static_cast<float>(mag_raw_msb.y);
  mag.z = static_cast<float>(mag_raw_msb.z);
  mag *= _inv_mag_sens;

  return true;
}
