---
id: "0005"
title: WS2812B Hardware-Pure Layer 1 HAL, Layer 2 Minimal Driver & Future Status Presets Roadmap
author: Drone Research Team / Developer
date: 2026-09-17
time: 20:00:00 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#ws2812b", "#led", "#roadmap"]
references:
  - "[[0000-firmwareStructureLayerDecision]]"
  - "[[0002-taskEnumsAndHalAbstractionDecision]]"
  - "[[0003-simdHalArchitectureAndLayeringRule]]"
---

# Zettel 0005: WS2812B Hardware-Pure Layer 1 HAL, Layer 2 Minimal Driver & Future Status Presets Roadmap

## 1. Metadata & Context
- **ID**: `0005`
- **Author**: Drone Research Team / Developer
- **Date**: 2026-09-17
- **Time**: 20:00:00 +07:00
- **Status**: Approved & Implemented
- **References**: `[Note 0000]`, `[Note 0002]`, `[Note 0003]`

---

## 2. Core Architectural Decisions

### 2.1 Hardware-Pure Layer 1 RMT Driver (`hal_rgb_led.h`, `rgb_led_rmt.cpp`)
- Per **Zettel 0003**, Layer 1 HAL drivers must be hardware-pure drivers without scalar `#else` fallbacks or unnecessary higher-level state logic.
- `HAL_RGB_LED` provides the pure target interface, implemented for ESP32-S3 via `RGB_LED_RMT` using the hardware RMT transceiver on GPIO 47 (`RGB_LED_PIN`).

### 2.2 Minimal Layer 2 WS2812B Peripheral Driver (`ws2812b.h`, `ws2812b.cpp`)
- To prevent unnecessary RTOS task overhead and retain flexible usage across different subsystems, no dedicated LED FreeRTOS task is spawned.
- Layer 2 driver provides core direct API primitives:
  - `void init(uint8_t pin = RGB_LED_PIN)`
  - `void set_color(uint8_t r, uint8_t g, uint8_t b)`
  - `void set_hex(uint32_t hex_color)`
  - `void off()`

---

## 3. Future Development Roadmap: System Status LED Presets

The high-level status indication color mappings below were decoupled from the minimal driver implementation and deferred to future FSM / status management system development:

```cpp
// Reserved for future FSM / Status Management integration:
// led.set_status_booting();       -> Solid Blue (System initialization)
// led.set_status_disarmed();      -> Solid Green (FC ready / disarmed)
// led.set_status_armed();         -> Solid Red (FC armed)
// led.set_status_error();         -> Bright Magenta / Red (Sensor error / Failsafe)
// led.set_status_ble_connected(); -> Solid Cyan (Bluetooth connected)
```

Future status preset logic will be handled either by the central Finite State Machine (FSM) module (`src/app/fsm.cpp`) or a dedicated status notification service when full flight mode handling is integrated.
