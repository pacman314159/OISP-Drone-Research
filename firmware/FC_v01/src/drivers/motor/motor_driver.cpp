#include "drivers/motor/motor_driver.h"
#include <cmath>

MotorDriver::MotorDriver(HAL_PWM& pwm_hal) : _pwm_hal(pwm_hal), _initialized(false){
  for(uint8_t i = 0; i < MOTOR_COUNT; ++i) _current_norm[i] = 0.0f;
}

bool MotorDriver::init(const int gpio_pins[MOTOR_COUNT], uint32_t freq_hz, uint8_t res_bits){
  if(gpio_pins == nullptr) return false;

  if(!_pwm_hal.init_timer(freq_hz, res_bits)) return false;

  for(uint8_t i = 0; i < MOTOR_COUNT; ++i){
    if(!_pwm_hal.init_channel(i, gpio_pins[i])) return false;
  }

  _initialized = true;
  disarm_all();
  return true;
}

void MotorDriver::set_motor_normalized(uint8_t motor_idx, float throttle_norm){
  if(!_initialized || !_armed || motor_idx >= MOTOR_COUNT) return;

  // Numerical sanity and boundary clamping to [0.0, 1.0]
  if(!std::isfinite(throttle_norm) || throttle_norm < 0.0f)
    throttle_norm = 0.0f;
  else if(throttle_norm > 1.0f)
    throttle_norm = 1.0f;

  _current_norm[motor_idx] = throttle_norm;

  // Linear mapping: [0.0, 1.0] -> [ESC_PWM_MIN_US, ESC_PWM_MAX_US] (1000 us to 2000 us)
  const float pulse_span = static_cast<float>(ESC_PWM_MAX_US - ESC_PWM_MIN_US);
  uint16_t pulse_us = static_cast<uint16_t>(ESC_PWM_MIN_US + (throttle_norm * pulse_span));

  _pwm_hal.set_pulse_us(motor_idx, pulse_us);
}

void MotorDriver::set_all_normalized(const float throttle_norm[MOTOR_COUNT]){
  if(!_initialized || throttle_norm == nullptr) return;

  for(uint8_t i = 0; i < MOTOR_COUNT; ++i)
    set_motor_normalized(i, throttle_norm[i]);
}

void MotorDriver::arm_all(){
  _armed = true;
}

void MotorDriver::disarm_all(){
  _armed = false;
  for(uint8_t i = 0; i < MOTOR_COUNT; ++i){
    _current_norm[i] = 0.0f;
    _pwm_hal.set_pulse_us(i, ESC_PWM_DISARMED_US);
  }
}

bool MotorDriver::is_armed() const {
  return _armed;
}

float MotorDriver::get_motor_normalized(uint8_t motor_idx) const {
  if(motor_idx >= MOTOR_COUNT) return 0.0f;
  return _current_norm[motor_idx];
}

