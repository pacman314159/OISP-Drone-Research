#pragma once
#include <cstdint>
#include "config.h"

class HAL_RGB_LED {
public:
  virtual ~HAL_RGB_LED() = default;
  virtual void init(uint8_t gpio_pin) = 0;
  virtual void set_pixel(uint8_t r, uint8_t g, uint8_t b) = 0;
  virtual void show() = 0;
};

// Global Target LED Accessor (Pure HAL interface, target-decoupled)
HAL_RGB_LED& get_rgb_led();
