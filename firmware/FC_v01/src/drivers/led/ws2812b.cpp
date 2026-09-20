#include "drivers/led/ws2812b.h"

WS2812B::WS2812B(HAL_RGB_LED& hal_led)
  : _hal_led(hal_led){}

void WS2812B::init(uint8_t pin){
  _hal_led.init(pin);
  off();
}

void WS2812B::set_color(uint8_t r, uint8_t g, uint8_t b){
  _hal_led.set_pixel(r, g, b);
  _hal_led.show();
}

void WS2812B::set_hex(uint32_t hex_color){
  uint8_t r = static_cast<uint8_t>((hex_color >> 16) & 0xFF);
  uint8_t g = static_cast<uint8_t>((hex_color >> 8) & 0xFF);
  uint8_t b = static_cast<uint8_t>(hex_color & 0xFF);
  set_color(r, g, b);
}

void WS2812B::off(){
  set_color(0, 0, 0);
}

