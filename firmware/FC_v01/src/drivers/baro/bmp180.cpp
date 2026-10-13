#include "drivers/baro/bmp180.h"

BMP180::BMP180(HAL_I2C_BUS& i2c_bus, uint8_t dev_addr)
  : _i2c(i2c_bus),
    _dev_addr(dev_addr),
    _oss(BMP_OSS_HIGH_RES),
    _calib{},
    _b5(0){}

bool BMP180::init(){
  uint8_t chip_id = 0;
  if(!_i2c.read_reg(_dev_addr, REG_CHIP_ID, &chip_id) || chip_id != BMP180_CHIP_ID) return false;
  if(!read_calibration_data()) return false;

  return true;
}

bool BMP180::read_calibration_data(){
  uint8_t buf[22] = {0};
  if(!_i2c.read_reg_multi(_dev_addr, REG_CALIB_START, buf, 22)) return false;

  _calib.ac1 = static_cast<int16_t>((buf[0] << 8) | buf[1]);
  _calib.ac2 = static_cast<int16_t>((buf[2] << 8) | buf[3]);
  _calib.ac3 = static_cast<int16_t>((buf[4] << 8) | buf[5]);
  _calib.ac4 = static_cast<uint16_t>((buf[6] << 8) | buf[7]);
  _calib.ac5 = static_cast<uint16_t>((buf[8] << 8) | buf[9]);
  _calib.ac6 = static_cast<uint16_t>((buf[10] << 8) | buf[11]);
  _calib.b1  = static_cast<int16_t>((buf[12] << 8) | buf[13]);
  _calib.b2  = static_cast<int16_t>((buf[14] << 8) | buf[15]);
  _calib.mb  = static_cast<int16_t>((buf[16] << 8) | buf[17]);
  _calib.mc  = static_cast<int16_t>((buf[18] << 8) | buf[19]);
  _calib.md  = static_cast<int16_t>((buf[20] << 8) | buf[21]);

  if(_calib.ac4 == 0 || _calib.ac5 == 0 || _calib.ac6 == 0 || _calib.md == 0) return false;

  return true;
}

bool BMP180::start_temperature_meas(){
  return _i2c.write_reg(_dev_addr, REG_CTRL_MEAS, CMD_READ_TEMP);
}

bool BMP180::read_temperature_raw(int32_t& ut){
  uint8_t buf[2] = {0};
  if(!_i2c.read_reg_multi(_dev_addr, REG_OUT_MSB, buf, 2)) return false;

  ut = static_cast<int32_t>((buf[0] << 8) | buf[1]);
  calculate_b5(ut);

  return true;
}

bool BMP180::start_pressure_meas(BmpOss oss){
  _oss = oss;
  uint8_t cmd = CMD_READ_PRESS + (static_cast<uint8_t>(oss) << 6);
  return _i2c.write_reg(_dev_addr, REG_CTRL_MEAS, cmd);
}

bool BMP180::read_pressure_raw(int32_t& up){
  uint8_t buf[3] = {0};
  if(!_i2c.read_reg_multi(_dev_addr, REG_OUT_MSB, buf, 3)) return false;

  up = ((static_cast<int32_t>(buf[0]) << 16) | (static_cast<int32_t>(buf[1]) << 8) | static_cast<int32_t>(buf[2])) >> (8 - static_cast<uint8_t>(_oss));

  return true;
}

int32_t BMP180::calculate_b5(int32_t ut){
  int32_t x1 = ((ut - static_cast<int32_t>(_calib.ac6)) * static_cast<int32_t>(_calib.ac5)) >> 15;
  int32_t denom = x1 + static_cast<int32_t>(_calib.md);
  if(denom == 0) return _b5;

  int32_t x2 = (static_cast<int32_t>(_calib.mc) << 11) / denom;
  _b5 = x1 + x2;
  return _b5;
}

float BMP180::calculate_temperature(int32_t ut){
  int32_t b5 = calculate_b5(ut);
  return static_cast<float>((b5 + 8) >> 4) * 0.1f;
}

float BMP180::calculate_pressure(int32_t up, BmpOss oss){
  int32_t b6 = _b5 - 4000;
  int32_t x1 = (static_cast<int32_t>(_calib.b2) * ((b6 * b6) >> 12)) >> 11;
  int32_t x2 = (static_cast<int32_t>(_calib.ac2) * b6) >> 11;
  int32_t x3 = x1 + x2;

  uint8_t oss_val = static_cast<uint8_t>(oss);
  int32_t b3 = (((static_cast<int32_t>(_calib.ac1) * 4 + x3) << oss_val) + 2) / 4;

  x1 = (static_cast<int32_t>(_calib.ac3) * b6) >> 13;
  x2 = (static_cast<int32_t>(_calib.b1) * ((b6 * b6) >> 12)) >> 16;
  x3 = ((x1 + x2) + 2) >> 2;

  uint32_t b4 = (static_cast<uint32_t>(_calib.ac4) * static_cast<uint32_t>(x3 + 32768)) >> 15;
  if(b4 == 0) return 0.0f;

  uint32_t b7 = (static_cast<uint32_t>(up) - b3) * (50000 >> oss_val);
  int32_t p = 0;
  if(b7 < 0x80000000){
    p = (b7 * 2) / b4;
  }else{
    p = (b7 / b4) * 2;
  }

  x1 = (p >> 8) * (p >> 8);
  x1 = (x1 * 3038) >> 16;
  x2 = (-7357 * p) >> 16;
  p = p + ((x1 + x2 + 3791) >> 4);

  return static_cast<float>(p);
}

uint32_t BMP180::get_conversion_delay_ms(BmpOss oss){
  switch(oss){
    case BMP_OSS_ULTRA_LOW_POWER: return 5;
    case BMP_OSS_STANDARD:        return 8;
    case BMP_OSS_HIGH_RES:        return 14;
    case BMP_OSS_ULTRA_HIGH_RES:  return 26;
    default:                      return 14;
  }
}

float BMP180::pressure_to_altitude(float pressure_pa, float sea_level_pa){
  if(pressure_pa <= 0.0f || sea_level_pa <= 0.0f) return 0.0f;
  return 44330.0f * (1.0f - std::pow(pressure_pa / sea_level_pa, 0.190295f));
}
