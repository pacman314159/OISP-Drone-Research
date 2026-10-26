#pragma once
#include <cstdint>
#include <cstddef>

class HAL_PWM {
public:
  virtual ~HAL_PWM() = default;
  virtual bool init_timer(uint32_t freq_hz, uint8_t resolution_bits) = 0;
  virtual bool init_channel(uint8_t channel_idx, int gpio_pin, uint32_t initial_duty_ticks = 0) = 0;
  virtual bool set_duty_ticks(uint8_t channel_idx, uint32_t duty_ticks) = 0;
  virtual bool set_pulse_us(uint8_t channel_idx, uint16_t pulse_us) = 0;
  virtual bool stop_channel(uint8_t channel_idx) = 0;
  virtual uint32_t get_freq_hz() const = 0;
  virtual uint8_t get_resolution_bits() const = 0;
};

// Global Target PWM Accessor (Pure HAL interface, defined per platform target build)
HAL_PWM& get_pwm_hal();

