#pragma once
#include <cstdint>
#include "config.h"
#include "platforms/hal/hal_pwm.h"

// =============================================================================
// LAYER 2 SENSOR & PERIPHERAL DRIVERS - MOTOR ESC ACTUATOR DRIVER
// =============================================================================

class MotorDriver {
public:
  explicit MotorDriver(HAL_PWM& pwm_hal);

  bool init(const int gpio_pins[MOTOR_COUNT], uint32_t freq_hz = ESC_PWM_FREQ_HZ, uint8_t res_bits = ESC_LEDC_TIMER_BIT_DEPTH);
  void set_motor_normalized(uint8_t motor_idx, float throttle_norm); // Normalized throttle in [0.0, 1.0]
  void set_all_normalized(const float throttle_norm[MOTOR_COUNT]);   // 4 normalized motor signals in [0.0, 1.0]
  void arm_all();                                                    // Enable motor output state
  void disarm_all();                                                 // Immediate safety override to disarmed pulse (1000 us)
  bool is_armed() const;                                             // True if actuator is armed
  float get_motor_normalized(uint8_t motor_idx) const;

private:
  HAL_PWM& _pwm_hal;
  float    _current_norm[MOTOR_COUNT]{0.0f, 0.0f, 0.0f, 0.0f}; // Current normalized commands in [0.0, 1.0]
  bool     _initialized{false};
  bool     _armed{false};
};

