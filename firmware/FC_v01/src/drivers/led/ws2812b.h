#pragma once
#include <cstdint>
#include "platforms/hal/hal_rgb_led.h"
#include "config.h"

class WS2812B {
public:
  explicit WS2812B(HAL_RGB_LED& hal_led);

  void init(uint8_t pin = RGB_LED_PIN);
  void set_color(uint8_t r, uint8_t g, uint8_t b);
  void set_hex(uint32_t hex_color);
  void off();


private:
  HAL_RGB_LED& _hal_led;
};
