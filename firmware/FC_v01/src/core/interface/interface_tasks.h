#pragma once
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "core/interface/user_button.h"
#include "drivers/led/ws2812.h"

// =============================================================================
// LAYER 4 INTERFACE TASK RUNNERS & TYPES
// =============================================================================

// RGB LED Subsystem Command Structures (FSM-exclusive Producer)
enum LedPattern : uint8_t {
  LED_PATTERN_OFF = 0,
  LED_PATTERN_SOLID,
  LED_PATTERN_BLINK_SLOW,
  LED_PATTERN_BLINK_FAST,
  LED_PATTERN_BREATHE,
  LED_PATTERN_FLASH_BURST
};

struct LedCommand {
  LedPattern     pattern;
  PaletteColorId color;
};

// WS2812 Single RGB Status LED Pattern & Display Task Runner ([Note 0023])
void status_led_task(void* arg);
