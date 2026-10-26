#pragma once
#include "platforms/hal/hal_pwm.h"
#include <driver/ledc.h>

class ESP32_PWM_LEDC : public HAL_PWM {
private:
  uint32_t _freq_hz{250};        // Carrier frequency in Hertz (Hz)
  uint8_t  _resolution_bits{14}; // Timer resolution in bits (max 14 bits on ESP32-S3)
  bool     _timer_initialized{false};
  bool     _channel_initialized[8]{false, false, false, false, false, false, false, false};

public:
  ESP32_PWM_LEDC() = default;
  ~ESP32_PWM_LEDC() override = default;

  bool init_timer(uint32_t freq_hz, uint8_t resolution_bits) override;
  bool init_channel(uint8_t channel_idx, int gpio_pin, uint32_t initial_duty_ticks = 0) override;
  bool set_duty_ticks(uint8_t channel_idx, uint32_t duty_ticks) override;
  bool set_pulse_us(uint8_t channel_idx, uint16_t pulse_us) override;
  bool stop_channel(uint8_t channel_idx) override;
  uint32_t get_freq_hz() const override { return _freq_hz; }
  uint8_t get_resolution_bits() const override { return _resolution_bits; }
};

