---
id: "0012"
title: WS2812 External Hardware Treatment & Mutex Encapsulation Rule
author: Drone Research Team / Developer
date: 2026-10-03
time: 01:47:00 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#ws2812", "#mutex"]
references:
  - "[[0000-firmwareStructureLayerDecision]]"
  - "[[0002-taskEnumsAndHalAbstractionDecision]]"
---

# Zettel 0012: WS2812 External Hardware Treatment & Mutex Encapsulation Rule

## 1. Prime Directive: WS2812 is an External Hardware Peripheral
The onboard WS2812 addressable RGB LED must be treated strictly as an **external physical hardware device**, identical in standing to `MPU6050` and `HMC5883L`:
- It is NOT an application utility, nor a driver singleton.
- It is instantiated statically in Layer 5 (`src/app/app.cpp`) using `get_pulse_tx0()`, per `[Note 0000]` and `[Note 0002]`.
- Subsystems and callbacks receive `WS2812*` pointers explicitly; global singletons in Layer 1 or Layer 2 are forbidden.

## 2. Mandatory Mutex Encapsulation Rule
Because the LED is a shared hardware resource accessed across multiple asynchronous tasks (e.g. Telemetry/BLE, Flight Core State, Battery Monitor):
- A dedicated mutex (`status_led_mutex`) is allocated in Layer 3 (`src/middleware/ipc.h`).
- **All interactions** with the driver (`init()`, `set_rgb()`, `set_brightness()`, `set_high()`, `set_low()`) **MUST be encapsulated between `xSemaphoreTake()` and `xSemaphoreGive()`**:

```cpp
if(xSemaphoreTake(status_led_mutex, pdMS_TO_TICKS(BUS_MUTEX_TIMEOUT_MS)) == pdTRUE){
  status_led.set_rgb(r, g, b);
  xSemaphoreGive(status_led_mutex);
}
```

Direct, un-synchronized access without `status_led_mutex` is strictly prohibited across all layers.
