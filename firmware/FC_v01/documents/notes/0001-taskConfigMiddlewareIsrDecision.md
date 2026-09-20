---
id: "0001"
title: Centralized Task Config, Middleware ISR Callbacks & RMT PPM Decoding Decision
author: Drone Research Team / Developer
date: 2026-09-15
time: 19:43:36 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#freertos", "#config", "#espressif", "#rmt"]
references:
  - "[[0000-firmwareStructureLayerDecision]]"
---

# Zettel 0001: Centralized Task Config, Middleware ISR Callbacks & RMT PPM Decoding Decision

## 1. Metadata & Context
- **ID**: `0001`
- **Author**: Drone Research Team / Developer
- **Date**: 2026-09-15
- **Time**: 19:43:36 +07:00
- **Status**: Approved & Enforced
- **References**: `[Note 0000]` (5-Layer Architectural Structure)

---

## 2. Core Decisions & Roadmap

### A. Centralized RTOS Task Parameters (`include/config.h`)
- All FreeRTOS task configuration parameters — including task names, stack sizes (in words/bytes), task priority levels (`TASK_PRIORITY_*`), target loop frequencies (e.g. 500 Hz IMU rate), and core pinning assignments — MUST be declared centrally inside `include/config.h`.
- Hardcoding magic numbers for task stack sizes or priorities inside individual task spawning calls (e.g. `xTaskCreatePinnedToCore`) is strictly forbidden across all layers.

### B. Middleware Layer Isolation for ISRs & Callbacks
- All hardware Interrupt Service Routines (ISRs) and asynchronous driver completion callbacks (such as I2C async transaction callbacks, hardware timer interrupt handlers, and RMT frame interrupts) MUST be defined and managed within Layer 3 (`src/middleware/`).
- This isolates low-level hardware interrupt handling and FreeRTOS ISR notification primitives from high-level driver and application business logic.

### C. Hardware RMT Peripheral for PPM RC RX Decoding (Espressif Architecture)
- RC receiver PPM (Pulse Position Modulation) signal decoding tasks on Espressif targets (ESP32-S3) MUST be implemented using the built-in **RMT (Remote Control Transceiver)** hardware peripheral.
- Utilizing hardware RMT pulse edge timing hardware offloads pulse width measurement from the main CPU cores, preventing CPU interrupt starvation and preserving CPU clock cycles for the primary 500 Hz flight control loop.

### D. Centralized I2C Bus Mutex Synchronization Rule
- All I2C hardware bus transactions invoked in the driver layer from the core layer (e.g. reading MPU6050, HMC5883L, BMP180, or power monitor data) MUST be strictly bounded by `xSemaphoreTake()` and `xSemaphoreGive()` using the centralized FreeRTOS bus mutex handles (`i2c0_mutex` for Bus 0, `i2c1_mutex` for Bus 1) declared in Layer 3 (`src/middleware/`).
- Direct un-synchronized driver access to shared hardware I2C buses without acquiring the corresponding middleware mutex is strictly prohibited.

---

## 3. Impact & Architectural Rules
1. **Config Centralization**: Check `include/config.h` before adding any new RTOS task. All task stack sizes and priorities must be referenced via `config.h` macros.
2. **ISR Location Rule**: Do not put raw `ISR_ATTR` interrupt handler functions inside `src/app/` or `src/drivers/`. Route hardware interrupts through `src/middleware/`.
3. **RMT PPM Resource Allocation**: Reserve ESP32-S3 RMT RX channel 0 specifically for PPM receiver signal timing capture.
4. **I2C Mutex Bounding Rule**: Enforce `xSemaphoreTake(i2c0_mutex, ...)` / `xSemaphoreGive(i2c0_mutex)` (or `i2c1_mutex`) around all I2C driver data transactions initiated by core tasks.

