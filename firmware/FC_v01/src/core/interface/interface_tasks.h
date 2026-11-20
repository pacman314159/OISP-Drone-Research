#pragma once
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "core/interface/user_button.h"
#include "drivers/led/ws2812.h"
#include "ipc_types.h"   // LedPattern / LedCommand contract (C15)

// =============================================================================
// LAYER 4 INTERFACE TASK RUNNERS & TYPES
// =============================================================================

// WS2812 Single RGB Status LED Pattern & Display Task Runner ([Note 0023])
void status_led_task(void* arg);
