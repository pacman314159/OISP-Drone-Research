---
id: "0033"
title: PlatformIO Toolchain Package Pruning, Arduino Framework Scope, and Multi-Generation Dependency Audit
author: Drone Research Team / Developer
date: 2026-10-22
time: 16:42:00 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#platformio", "#toolchain", "#arduino", "#espidf", "#nimble", "#dependencies"]
references:
  - "[[0000-firmwareStructureLayerDecision]]"
  - "[[0004-simdMultiTypeSupportAndBuildIntegration]]"
  - "[[0007-systemViewTracingAndRTOSTimingAnalysisConventions]]"
  - "[[0015-bleOtaArchitectureHardeningAndResourceCleanup]]"
  - "[[0021-bleTelemetryDispatcherConsolidationAndSubscriptionGating]]"
---

# Zettel 0033: PlatformIO Toolchain Package Pruning, Arduino Framework Scope, and Multi-Generation Dependency Audit

## 1. Metadata & Scope
- **ID**: `0033`
- **Author**: Drone Research Team / Developer
- **Date**: 2026-10-22
- **Time**: 16:42:00 +07:00
- **Status**: Verified & Audited
- **References**: `[Note 0000]`, `[Note 0004]`, `[Note 0007]`, `[Note 0015]`, `[Note 0021]`

---

## 2. Problem Statement & Context
During disk sanitization of the global PlatformIO package cache (`C:\Users\Admin\.platformio\packages`), ~1.98 GB was reclaimed by removing two redundant framework directories:
1. `framework-arduinoespressif32` (735.8 MB)
2. `framework-arduinoespressif32@src-c123a88196734796ed4f6cde1417e7b5` (1,246.8 MB - orphaned Git clone snapshot)

A codebase audit across the knowledge graph (Graphify) and architectural records (Zettelkasten) was executed to verify which modules, tasks, and libraries depend on the Arduino framework and whether any latent dependencies were tied to the deprecated snapshots.

---

## 3. Dependency Mapping & Graphify Subgraph Analysis

Through Graphify traversal of `FC_v01`, the Arduino and NimBLE component dependencies are isolated strictly to Layer 0 (Entrypoint), Layer 2 (Drivers), and Layer 4 (Telemetry Core):

```
platformio.ini [framework = arduino, espidf]
      │
      ├──> src/main.cpp
      │       ├── initArduino()
      │       └── Serial.begin(115200)
      │
      ├──> lib_deps: h2zero/NimBLE-Arduino@^1.4.1
      │       │
      │       └──> src/drivers/telemetry/ble_driver.cpp
      │               ├── #include <NimBLEDevice.h>
      │               ├── GATT Characteristic Registration
      │               └── Dual-Trigger CCCD Notification
      │
      └──> Core Telemetry Tasks (Consuming BLE Driver APIs)
              ├── src/core/telemetry/ble_telem_tasks.cpp
              └── src/core/telemetry/ble_ota_tasks.cpp
```

### Affected Translation Units & Architectural Constraints:
1. **`src/main.cpp` (Layer 0 Entrypoint)**:
   - Invokes `initArduino()` to initialize Arduino HAL timers and serial buffers while running inside the native ESP-IDF `app_main()` task.
2. **`src/drivers/telemetry/ble_driver.cpp` (Layer 2 Driver)**:
   - Relies on `NimBLEDevice.h` (wrapped for Arduino-ESP32).
   - Operates under `Zettel 0015` and `Zettel 0021` for non-blocking queue dispatch and CCCD subscription gating.
3. **Core Tasks (`src/core/`)**:
   - Follow `Zettel 0000` 5-layer isolation: zero direct calls to Arduino headers inside sensor DAQ (`daq_tasks.cpp`), EKF attitude estimation (`estimator_tasks.cpp`), or FSM (`fsm.cpp`). All core tasks interface exclusively through abstract HAL interfaces (`hal_i2c.h`, `hal_spi.h`, `ipc.h`).

---

## 4. Audit Findings & Resolution

1. **Zero Hardcoded Package Dependencies**:
   - No project file, CMake configuration, or compiler script references explicit paths inside `~/.platformio/packages/`.
   - The removed `@src-c123...` folder was an unpinned, orphaned Git cache from an earlier experimental branch; no active code relied on it.
2. **Automated On-Demand Recovery**:
   - PlatformIO automatically downloads the official, stable release of `framework-arduinoespressif32` matching the active `platform = espressif32` specification upon the next manual build (`pio run`).
3. **Toolchain Alignment**:
   - The project builds for `esp32-s3-devkitc-1` and utilizes the newly installed `toolchain-xtensa-esp-elf` (1.35 GB, installed 2026-10-22).
   - Retained modern RISC-V toolchains (`toolchain-riscv32-esp`) ensure future ESP32-C3 / ESP32-C6 targets remain instantly operational without compiler re-fetching.

