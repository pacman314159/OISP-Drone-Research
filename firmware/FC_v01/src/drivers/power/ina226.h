#pragma once
#include <cstdint>
#include <cstddef>
#include "config.h"

// Conditional HAL abstraction selector (Sync when SYSTEM_I2C1_MODE_ASYNC is false)
#if (!SYSTEM_I2C1_MODE_ASYNC)
  #include "platforms/hal/hal_i2c.h"
  using HAL_I2C_BUS = HAL_I2C;
#else
  #include "platforms/hal/hal_i2c_async.h"
  using HAL_I2C_BUS = HAL_I2C_ASYNC;
#endif

// =============================================================================
// INA226 REGISTER DEFINITIONS & CHIP IDENTIFIERS
// =============================================================================
constexpr uint8_t  INA226_DEFAULT_ADDR     = 0x40;
constexpr uint16_t INA226_MANUFACTURER_ID  = 0x5449; // "TI"
constexpr uint16_t INA226_DIE_ID_VAL       = 0x2260;

// INA226 Hardware Register Address Map
enum Ina226Register : uint8_t {
  INA_REG_CONFIG       = 0x00,
  INA_REG_SHUNT_V      = 0x01,
  INA_REG_BUS_V        = 0x02,
  INA_REG_POWER        = 0x03,
  INA_REG_CURRENT      = 0x04,
  INA_REG_CALIBRATION  = 0x05,
  INA_REG_MASK_ENABLE  = 0x06,
  INA_REG_ALERT_LIMIT  = 0x07,
  INA_REG_MANUF_ID     = 0xFE,
  INA_REG_DIE_ID       = 0xFF
};

// Configuration Enums (Modular architecture matching Mpu/Hmc driver style)
enum InaAvgMode : uint8_t {
  INA_AVG_1    = 0,
  INA_AVG_4    = 1,
  INA_AVG_16   = 2,
  INA_AVG_64   = 3,
  INA_AVG_128  = 4,
  INA_AVG_256  = 5,
  INA_AVG_512  = 6,
  INA_AVG_1024 = 7
};

enum InaConvTime : uint8_t {
  INA_CONV_140US  = 0,
  INA_CONV_204US  = 1,
  INA_CONV_332US  = 2,
  INA_CONV_588US  = 3,
  INA_CONV_1100US = 4,
  INA_CONV_2116US = 5,
  INA_CONV_4156US = 6,
  INA_CONV_8244US = 7
};

enum InaMode : uint8_t {
  INA_MODE_POWER_DOWN     = 0,
  INA_MODE_SHUNT_TRIG     = 1,
  INA_MODE_BUS_TRIG       = 2,
  INA_MODE_SHUNT_BUS_TRIG = 3,
  INA_MODE_SHUTDOWN       = 4,
  INA_MODE_SHUNT_CONT     = 5,
  INA_MODE_BUS_CONT       = 6,
  INA_MODE_SHUNT_BUS_CONT = 7
};

// Configuration bitfield constants
constexpr uint16_t INA226_CONFIG_DEFAULT   = 0x4327; // AVG=4, VBUSCT=1.1ms, VSHCT=1.1ms, MODE=continuous
constexpr uint16_t INA226_CONFIG_RESET_BIT = 0x8000;

struct PowerSample {
  float voltage_mv; // Bus voltage in millivolts (mV)
  float current_ma; // Current in milliamps (mA)
  float power_mw;   // Power in milliwatts (mW)
  float shunt_uv;   // Shunt voltage in microvolts (uV)
};

// Power & Battery Measurement Data Structure (Millis Scale)
struct PowerMeasurement {
  float voltage_mv; // Bus voltage in millivolts (mV)
  float current_ma; // Current in milliamps (mA)
  float power_mw;   // Power in milliwatts (mW)
  float energy_mah; // Accumulated energy in milliamp-hours (mAh)
};

class INA226 {

public:
  explicit INA226(HAL_I2C_BUS& i2c_bus, uint8_t dev_addr = INA226_I2C_ADDR);

  bool init(float shunt_resistor_ohm = INA226_SHUNT_RESISTOR_OHM, float current_lsb_ma = INA226_CURRENT_LSB_MA);
  bool verify_chip_id();
  bool calibrate(uint16_t cal_val = INA226_CALIBRATION_VAL);

  bool set_averaging(InaAvgMode avg);
  bool set_bus_conversion_time(InaConvTime conv_time);
  bool set_shunt_conversion_time(InaConvTime conv_time);
  bool set_mode(InaMode mode);

  bool get_all_data_raw(uint16_t& raw_bus_v, int16_t& raw_shunt_v, int16_t& raw_current, uint16_t& raw_power);
  bool get_all_data(PowerSample& sample);
  bool read_sample(PowerSample& out_sample){ return get_all_data(out_sample); }
  bool is_initialized() const{ return _initialized; }
  bool is_connected() const{ return _initialized; }

  bool read_bus_voltage_mv(float& out_voltage_mv);
  bool read_current_ma(float& out_current_ma);
  bool read_power_mw(float& out_power_mw);
  bool read_shunt_voltage_uv(float& out_shunt_uv);

  uint16_t get_raw_bus_voltage(){ return _raw_bus_voltage; }
  int16_t  get_raw_shunt_voltage(){ return _raw_shunt_voltage; }
  int16_t  get_raw_current(){ return _raw_current; }
  uint16_t get_raw_power(){ return _raw_power; }

private:
  bool write_register(uint8_t reg_addr, uint16_t value);
  bool read_register(uint8_t reg_addr, uint16_t& out_value);

  HAL_I2C_BUS& _i2c;
  uint8_t      _dev_addr;
  float        _shunt_resistor_ohm; // Shunt resistor in Ohms
  float        _current_lsb_ma;     // Current LSB in mA
  float        _power_lsb_mw;       // Power LSB in mW
  bool         _initialized;

  // Cached raw data registers
  uint16_t     _raw_bus_voltage;
  int16_t      _raw_shunt_voltage;
  int16_t      _raw_current;
  uint16_t     _raw_power;
};
