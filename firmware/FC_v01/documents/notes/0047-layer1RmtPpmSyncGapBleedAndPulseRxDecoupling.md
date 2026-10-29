---
id: "0047"
title: Layer 1 Hardware RMT PulseRX PPM Bleed & Layer 2 Protocol Decoupling Analysis
author: Drone Research Team / Developer
date: 2026-10-28
time: 01:25:00 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#rmt", "#ppm", "#rc_rx", "#hal", "#layering", "#technical-debt"]
references:
  - "[[0000-firmwareStructureLayerDecision]]"
  - "[[0030-rmtHardwarePpmDecodingFsIa6bAndBleTelemetryArchitecture]]"
---

# Zettel 0047: Layer 1 Hardware RMT PulseRX PPM Bleed & Layer 2 Protocol Decoupling Analysis

## 1. Metadata & Scope
- **ID**: `0047`
- **Author**: Drone Research Team / Developer
- **Date**: 2026-10-28
- **Time**: 01:25:00 +07:00
- **Status**: Documented Technical Debt & Deferred Architectural Plan
- **Module Context**: Layer 1 Hardware RMT (`src/platforms/targets/espressif/pulse_rmt.cpp`, `src/platforms/hal/hal_pulse.h`), Layer 2 RC Driver (`src/drivers/rc_rx/fs_ia6b.h`, `src/drivers/rc_rx/fs_ia6b.cpp`)

---

## 2. Problem Statement: Protocol Logic Bleed into Layer 1 HAL
Per the 5-layer system architecture (`documents/notes/0000-firmwareStructureLayerDecision.md`), **Layer 1 (HAL & Platform Targets)** must provide generic hardware abstractions without baking in specific higher-level protocol semantics.

Currently, `ESP32_PulseRX::rx_done_callback()` in `src/platforms/targets/espressif/pulse_rmt.cpp` contains hardcoded PPM radio frame decoding rules:

```cpp
// src/platforms/targets/espressif/pulse_rmt.cpp (Layer 1 Target)
bool IRAM_ATTR ESP32_PulseRX::rx_done_callback(...){
  ...
  // Hardcoded FlySky PPM sync-gap detection (> 3000 us)
  if(d0 >= 3000 || d1 >= 3000){
    if(frame.count >= 4) break;
    frame.count = 0;
    continue;
  }

  // Hardcoded PPM channel duration constraints (800 - 2200 us)
  uint32_t pulse = d0 + d1;
  if(pulse >= 800 && pulse <= 2200 && frame.count < 12){
    frame.pulses[frame.count++] = pulse;
  }
  ...
}
```

### Architectural Deficiencies
1. **Loss of HAL Generality**: `HAL_PulseRX` is intended to be a reusable pulse measurement peripheral. By hardcoding PPM framing, it cannot be reused for non-PPM pulse trains (e.g. ultrasonic distance sensors, tachometers, PWM capture).
2. **Duplicated Verification Logic**: In Layer 2, `FS_iA6B::update()` (`src/drivers/rc_rx/fs_ia6b.cpp`) re-filters the exact same parameters:
   ```cpp
   // src/drivers/rc_rx/fs_ia6b.cpp (Layer 2 Driver)
   if(p >= PPM_MIN_CHANNEL_US && p <= PPM_MAX_CHANNEL_US){
     _last_channels.channels[valid_count] = static_cast<uint16_t>(p);
     valid_count++;
   }
   ```
   This duplicates domain constraints across two separate architectural boundaries.

---

## 3. Proposed Future Decoupling Solution

When refactoring the pulse input subsystem:
1. **Layer 1 HAL Purity**:
   `ESP32_PulseRX` should solely manage the ESP32-S3 RMT RX channel, copying raw high/low duration symbols into a DMA or FreeRTOS queue without interpreting pulse semantics or discarding symbols.
2. **Layer 2 Protocol Decoder**:
   `FS_iA6B` (or an intermediate `PpmDecoder` class in Layer 2) receives the raw duration stream, identifies the sync-gap pause ($\ge 3000\,\mu\text{s}$), unpacks the active channel pulses ($1000\text{--}2000\,\mu\text{s}$), and detects signal loss / failsafe conditions.
3. **Status**:
   Retained as documented technical debt. The current implementation operates reliably at 50 Hz with zero interrupt jitter; decoupling will be scheduled when adding secondary pulse peripherals (e.g., optical flow / sonar).
