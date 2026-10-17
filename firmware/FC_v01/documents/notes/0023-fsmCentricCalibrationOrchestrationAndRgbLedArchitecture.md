---
id: "0023"
title: FSM-Centric Sensor Calibration Orchestration and RGB LED Status Architecture Specification
author: Drone Research Team / Developer
date: 2026-10-12
time: 18:50:00 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#fsm", "#freertos", "#led", "#calibration", "#ipc"]
references:
  - "[[0000-firmwareStructureLayerDecision]]"
  - "[[0001-taskConfigMiddlewareIsrDecision]]"
  - "[[0002-taskEnumsAndHalAbstractionDecision]]"
  - "[[0008-lpfAndCalibrationTaskArchitecture]]"
  - "[[0012-ws2812HardwareIsolationAndMutexEncapsulationRule]]"
  - "[[0020-fsmBackbonePreFlightSensorChecksAndActiveCalibrationArchitecture]]"
  - "[[0024-internalFlashNvsStorageHalAndSensorCalibrationPersistence]]"
  - "[[0025-rotEllipsoidMagCalibSimdAdamEkfUnitSphereStackLifecycle]]"
  - "[[0026-sixPointAccelCalibSimdEllipsoidFitFsmLedArchitecture]]"
  - "[[0031-sixGatePreflightTerminalDevelopmentAndTaskEncapsulationArchitecture]]"
---

# Zettel 0023: FSM-Centric Sensor Calibration Orchestration and RGB LED Status Architecture Specification

## 1. Metadata & Scope
- **ID**: `0023`
- **Author**: Drone Research Team / Developer
- **Date**: 2026-10-12
- **Status**: Approved & Implemented
- **References**: `[Note 0000]`, `[Note 0001]`, `[Note 0002]`, `[Note 0008]`, `[Note 0012]`, `[Note 0020]`

---

## 2. Problem Statement & Architecture Requirements
In previous iterations, RGB LED feedback was scattered across ad-hoc modules (direct driver mutations in BLE callbacks, startup setup, and SD card logging routines). Furthermore, calibration procedures lacked a formalized RTOS lifecycle architecture:
1. **Single Point of Authority for Visual Feedback**: All modules, tasks, and interrupt callbacks must be barred from mutating the RGB LED directly. Only the Finite State Machine (`TASK_FSM`) holds the authority to command LED behavior.
2. **Coordinator Design Pattern**: The FSM must exclusively supervise worker lifecycles. Calibration tasks must **never self-destruct** (`vTaskDelete(NULL)` is strictly forbidden). Instead, calibration tasks report healthchecks and completion status via IPC, halt passively, and delegate deletion to the FSM coordinator (`vTaskDelete(task_handle)`).
3. **Dual Calibration Categorization**:
   - **Singular Tasks (Gyro, Baro)**: Fixed sample acquisition series, averaging, and NVS storage.
   - **Iterative Tasks (Accel, Mag)**: Multi-step interactive human-in-the-loop acquisition (6-point tumble / spherical rotation) ending with numerical optimization (ellipsoid fitting).
4. **Non-Blocking RTOS Pacing**: Calibration tasks must operate in a `while()` loop paced by `vTaskDelayUntil` with anti-windup guards to ensure high-rate DAQ tasks (500 Hz IMU loop) have unhindered CPU access.

---

## 3. Core Architectural Decisions

### Rule 1: RGB LED Command Queue & Standalone Color Palette
- The WS2812 driver is driven by a dedicated, low-priority pattern worker: `TASK_STATUS_LED` (`src/core/interface/led_task.cpp`).
- The worker listens asynchronously to `led_cmd_queue` (`QueueHandle_t`), sleeping on `portMAX_DELAY` for static states (`LED_PATTERN_OFF`, `LED_PATTERN_SOLID`) and using 20 ms ticks for animated states (`LED_PATTERN_BLINK_SLOW`, `LED_PATTERN_BLINK_FAST`, `LED_PATTERN_BREATHE`, `LED_PATTERN_FLASH_BURST`).
- A 14-color standalone palette (`PaletteColorId`) provides discrete visual identifiers:
  - `PALETTE_CYAN`: Gyroscope
  - `PALETTE_ORANGE`: Accelerometer
  - `PALETTE_MAGENTA`: Magnetometer
  - `PALETTE_BLUE`: Barometer
  - `PALETTE_WHITE`: Acknowledgment / Step lock flash burst
  - `PALETTE_GREEN`: Calibration complete
  - `PALETTE_RED`: Failure / Abort indicator

### Rule 2: Inter-Process Communication & Healthcheck Protocol
- Worker tasks report to FSM via `calib_report_queue` (`QueueHandle_t`) passing `CalibReportMsg`:
  - `target`: `CALIB_TARGET_GYRO`, `CALIB_TARGET_ACCEL`, `CALIB_TARGET_MAG`, `CALIB_TARGET_BARO`
  - `status`: `CALIB_STATUS_HEALTHCHECK`, `CALIB_STATUS_STEP_LOCKED`, `CALIB_STATUS_COMPUTING`, `CALIB_STATUS_COMPLETED`, `CALIB_STATUS_FAILED`
  - `current_step`: Active step index (e.g., 0..5 for 6-point accel)
  - `progress_pct`: Percentage of routine completed
  - `timestamp_us`: Monotonic microsecond timestamp
- FSM drains `calib_report_queue` non-blockingly at 100 Hz.

### Rule 3: Task Spawning & Passive Halting Lifecycle
- User triggers calibration mode by holding the user button at power-on (`EVT_ACTIVE_CALIB_INVOKED`).
- FSM cycles through the 4 sensors on an RGB carousel (1.5s dwell).
- User clicks button to lock selection:
  - FSM sends white flash burst (`LED_PATTERN_FLASH_BURST`).
  - FSM spawns the corresponding task (`task_calib_gyro`, `task_calib_baro`, `task_calib_accel`, or `task_calib_mag`) via `xTaskCreatePinnedToCore`.
  - FSM sets LED to breathe with sensor color (`LED_PATTERN_BREATHE`).
- Upon routine completion:
  - Worker posts `CALIB_STATUS_COMPLETED` to `calib_report_queue`.
  - Worker enters passive halt via `ulTaskNotifyTake(pdTRUE, portMAX_DELAY)`.
  - FSM detects message, calls `vTaskDelete(s_active_calib_handle)`, clears the handle, and displays solid Green (`LED_PATTERN_SOLID`).
  - If `AUTO_REBOOT_AFTER_CALIBRATION == true`, FSM reboots after 2.0s; otherwise, parks holding solid Green.
- If user long-presses button (>2.0s), FSM aborts calibration, calls `vTaskDelete(s_active_calib_handle)`, flashes Red, and returns to selection menu.

---

## 4. Cross-References & Traceability
- Extends sensor calibration and state transition rules from `[[0020-fsmBackbonePreFlightSensorChecksAndActiveCalibrationArchitecture]]`.
- Concrete iterative worker implementation and 6-point pose capture gestures defined in `[[0026-sixPointAccelCalibSimdEllipsoidFitFsmLedArchitecture]]` and `[[0025-rotEllipsoidMagCalibSimdAdamEkfUnitSphereStackLifecycle]]`.
- Manages hardware WS2812 interface isolated per `[[0012-ws2812HardwareIsolationAndMutexEncapsulationRule]]`.

