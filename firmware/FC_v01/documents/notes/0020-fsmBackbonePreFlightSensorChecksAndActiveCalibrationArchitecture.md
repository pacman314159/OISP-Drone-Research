---
id: "0020"
title: Finite State Machine (FSM) Backbone, Pre-Flight Sensor Checks & Active Calibration Architecture Specification
author: Drone Research Team / Developer
date: 2026-10-10
time: 15:35:00 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#fsm", "#freertos", "#sensors", "#calibration"]
references:
  - "[[0000-firmwareStructureLayerDecision]]"
  - "[[0001-taskConfigMiddlewareIsrDecision]]"
  - "[[0002-taskEnumsAndHalAbstractionDecision]]"
  - "[[0006-taskCreationConfigEnumsAndTaskArgsPassingDecision]]"
  - "[[0008-lpfAndCalibrationTaskArchitecture]]"
  - "[[0012-ws2812HardwareIsolationAndMutexEncapsulationRule]]"
  - "[[0016-coreDirectoryCompactionAndDomainConsolidation]]"
  - "[[0018-sdCardLoggingRestorationAndUserButtonShutdown]]"
  - "[[0031-sixGatePreflightTerminalDevelopmentAndTaskEncapsulationArchitecture]]"
---

# Zettel 0020: Finite State Machine (FSM) Backbone, Pre-Flight Sensor Checks & Active Calibration Architecture Specification

## 1. Metadata & Scope
- **ID**: `0020`
- **Author**: Drone Research Team / Developer
- **Date**: 2026-10-10
- **Time**: 15:35:00 +07:00
- **Status**: Approved & Implemented
- **References**: `[Note 0000]`, `[Note 0002]`, `[Note 0006]`, `[Note 0008]`, `[Note 0012]`, `[Note 0016]`, `[Note 0018]`

---

## 2. Problem Statement & Architecture Requirements
Previous revisions lacked a unified flight state controller. Flight state transitions and pre-flight validations were scattered across ad-hoc startup logic. The system required:
1. **Deterministic State Lifecycle**: An asynchronous, deterministic FSM backbone managing transitions between uninitialized (`UNINIT`), calibration (`CALIB`), pre-flight checks (`PREFLIGHT`), armed readiness (`READY`), active flight (`TAKEOFF`, `FLIGHT`, `LANDING`), and emergency/failsafe states (`CRASHED`, `FREEFALL`, `LEVELOUT`).
2. **Deterministic Execution without Flight Loop Overhead**: The FSM must evaluate state transitions deterministically at **100 Hz** ($10\,\text{ms}$ loop period) using `vTaskDelayUntil`, while maintaining zero blocking or jitter impact on the 500 Hz DAQ and LPF flight loops.
3. **Emergency Wakeup**: Capable of zero-latency wakeups via FreeRTOS direct task notifications (`ulTaskNotifyTake`) when critical emergency events occur.
4. **Motor Disarming Safety Gate**: Motor outputs and control PID calculations must be gated strictly by `fsm_is_armed_allowed()` (`state == READY || state == FLIGHT || state == TAKEOFF`). In any other state, motor outputs must be zeroed.
5. **Pre-Flight Checks Pipeline**: Structured pre-flight verification covering stationary rest detection, passive calibration verification, and dedicated architectural hooks for tilt, sensor streams, RC signal, and storage checks.

---

## 3. Core Architectural Decisions

### Rule 1: Layer 5 Application Domain Scoping (`src/app/fsm.h`, `src/app/fsm.cpp`)
- The FSM coordinator resides in Layer 5 (Application Layer) as `fsm.h` and `fsm.cpp`.
- Centralized task parameters are declared in `include/config.h`:
  - `TASK_FSM_NAME = "FSM_MAIN"`
  - `TASK_FSM_ID` added to `enum TaskIDs`
  - `TASK_FSM_PRIORITY = 3` (Medium-high)
  - `TASK_FSM_STACK_SIZE = 4096`
  - `TASK_FSM_FREQ_HZ = 100` ($10\,\text{ms}$ period)
  - `TASK_FSM_CORE = tskNO_AFFINITY`

### Rule 2: Bitmask Events (`EVT_` Prefix) & Deterministic Transit Rules
- All system events follow the `EVT_` prefix in `enum SystemEventMasks`:
  - Calibration: `EVT_ACTIVE_CALIB_INVOKED`, `EVT_CALIB_COMPLETE`, `EVT_CALIB_FAILED`
  - Pre-flight checks: `EVT_STATIONARY_MET`, `EVT_PASSIVE_CALIB_PASS`, `EVT_PASSIVE_CALIB_FAIL`
  - Future checks: `EVT_TILT_CHECK_PASS/FAIL`, `EVT_SENSOR_STREAM_OK/FAIL`, `EVT_RC_SIGNAL_OK/FAIL`, `EVT_STORAGE_OK/FAIL`
  - Emergency: `EVT_EMERGENCY_SHUTDOWN`
- State transitions are declared via `TransitRule` entries:
  - `from_state`, `to_state`, `trigger_mask`, `neg_trigger_mask`, `trigger_policy`, `blocker_mask`, `block_policy`.
  - Trigger and blocker evaluations support `MatchPolicy::ALL`, `ANY`, `ALWAYS`, and `NEVER`.

### Rule 3: Active Calibration State (`CALIB`) Invocation
- The FSM task directly samples the hardware `USER_BTN_PIN` on startup.
- If held LOW during boot, the FSM sets `EVT_ACTIVE_CALIB_INVOKED` and executes `Rule_Uninit_To_Calib` to enter `CALIB`.
- Active calibration execution state is maintained asynchronously, avoiding startup blocking while other RTOS tasks initialize.

### Rule 4: Stationary Detection & Passive Calibration Verification (`PREFLIGHT`)
- In `PREFLIGHT`, the FSM reads shared IPC resources declared in `src/middleware/ipc.h`:
  1. **Stationary Detection**: Gathers raw IMU measurements directly from shared ring buffers (`accel_raw.front()`, `gyro_raw.front()`). Tracks raw sensor delta variance between consecutive 100 Hz frames. If variance stays below `STATIONARY_GYRO_VAR_DPS` ($0.8^\circ/\text{s}$) and `STATIONARY_ACCEL_VAR_MSS` ($0.15\,\text{m/s}^2$) continuously for `STATIONARY_TIME_REQUIRED_MS` ($1500\,\text{ms}$), `EVT_STATIONARY_MET` is asserted.
  2. **Passive Calibration Check**: Once stationary, evaluates the filtered sensor data stream (`gyro_filtered.front()`, `accel_filtered.front()`, `mag_filtered.front()`) which has not yet received calibrated offsets:
     - Filtered gyro zero-rate bias: $|g_{x,y,z}| < 2.0^\circ/\text{s}$ (`GYRO_PASSIVE_TOLERANCE_DPS`).
     - Filtered accel gravity magnitude: $|\|a\| - 1.0\text{g}| \times 9.80665 < 0.5\,\text{m/s}^2$ (`ACCEL_PASSIVE_TOLERANCE_MSS`).
     - Filtered mag field norm: $0.1 < \|m\| < 2.0$ Gauss.
  3. **Visual Error Latch**: If passive calibration fails, sets `EVT_PASSIVE_CALIB_FAIL`, permanently latches alternating Red/Amber flashes on the WS2812 status LED, prevents transitioning to `READY`, and disarms motor commands until system reboot, while allowing DAQ and telemetry tasks to continue running for diagnostic observation.

### Rule 5: Zero-Jitter Self-Gating Transition Engine (`evaluate_state_transitions`)
- The transition engine encapsulates its own fast-path gating check at its entry (`if(!s_events_dirty) return;`), liberating individual state handlers (`handle_state_ready()`, `handle_state_flight()`) from tracking or checking dirty flags.
- **Fast-Path Execution (< 1 µs)**: In steady flight or armed standby when no events or state changes have been registered, calling `evaluate_state_transitions()` immediately returns in a single branch check. This guarantees zero CPU contention or jitter on high-speed flight loops (500 Hz Rate/Attitude PID).
- **Sub-Millisecond Responsiveness**: Any external fail-safe, button ISR, or emergency trigger sets `s_events_dirty = true` and invokes `xTaskNotifyGive()`, ensuring immediate sub-millisecond transition execution.

### Rule 6: Layer 3 Button ISR Architecture & Decoupled Downward IPC Event Flow
- In compliance with the Layer 3 Middleware ISR routing rule and strict downward dependency hierarchy (`[Note 0000]`, `[Note 0001]`), `user_btn_isr()`, the static software hold timer (`s_btn_hold_timer`), and timer callback reside exclusively inside Layer 3 (`src/middleware/ipc.h`, `ipc.cpp`).
- Layer 3 never imports Layer 5 (`fsm.h`). Instead, Layer 3 exposes a thread-safe atomic event bitmask (`BtnEventMasks` in `ipc.h`) and `ipc_pop_btn_events()`. The ISR and timer callbacks set `BTN_EVT_SHORT_CLICK` / `BTN_EVT_LONG_HOLD` in Layer 3 and wake `TASK_FSM_ID` via `vTaskNotifyGiveFromISR()` / `xTaskNotifyGive()`.
- Layer 5 (`fsm_task`) polls/pops pending hardware button events via `ipc_pop_btn_events()` and translates them into system FSM events (`EVT_BTN_SHORT_CLICK`, `EVT_BTN_LONG_HOLD`), preserving total layer isolation.
- **Dual-Gesture Dispatcher**:
  1. **Short Click** ($50\ \text{ms} \le \Delta t \le 600\ \text{ms}$): Asserts `EVT_BTN_SHORT_CLICK`. In `CALIB` state, this selects/confirms active sensor calibration in the 1s RGB cycling menu.
  2. **Long Hold** ($\ge 3000\ \text{ms}$): Asserts `EVT_BTN_LONG_HOLD`.
- **Temporary State Bypass around `LANDING`**: The `LANDING` state enum is preserved in `SystemState`, but temporarily ignored/bypassed. A direct rule triggers `FLIGHT` $\rightarrow$ `POSTFLIGHT` on `EVT_BTN_LONG_HOLD`.
- **Post-Flight SD Logger Clean Shutdown**: Upon entering `POSTFLIGHT`, the FSM executes `handle_state_postflight()`, which asserts `sd_stop_logging_requested = true`, notifies `sd_logger_task` via `xTaskNotifyGive()`, flushes unwritten sectors, closes the file, and runs `trigger_sd_stop_led_feedback()` (3 cyan pulses) under `status_led_mutex`.

---

## 4. Remaining Work & Future Implementation Roadmap

The following modules and checks remain to be completed in upcoming implementation phases:

1. **Active Calibration Algorithms (`src/core/sensor/calib.cpp`)**:
   - Accelerometer 6-point multi-orientation ellipsoid fitting algorithm.
   - Magnetometer hard-iron and soft-iron ellipsoid calibration (`magcal()`).
   - Interactive WS2812 RGB menu (Red = Gyro, Green = Accel, Blue = Mag cycling every 1s, user button click to select).
   - ESP-IDF `nvs_flash` persistence to store `SensorCalibrationData` across reboots.
2. **Pre-Flight Tilt Check**:
   - Calculate platform pitch/roll from static gravity vector.
   - Assert `EVT_TILT_CHECK_PASS` if surface angle $< 10^\circ$, else `EVT_TILT_CHECK_FAIL`.
3. **Pre-Flight Sensor Stream Check**:
   - Monitor bus error counters (`i2c0_error_count`, etc.) and ensure timestamps for IMU, Mag, and Baro are incrementing within timeout bounds.
4. **Pre-Flight RC Controller Link & Failsafe Check**:
   - Verify PPM / S.Bus valid packet frame rate, failsafe bit clear, and throttle stick minimum position.
5. **Pre-Flight Storage / Blackbox Check**:
   - Verify SD card mount status, remaining sector capacity, and write latency before flight.
6. **Motor Mixer & Cascaded PID Arming Gate**:
   - Wire `fsm_is_armed_allowed()` into rate/attitude controller output stages to guarantee zero motor spin when not in `READY`, `TAKEOFF`, or `FLIGHT`.
7. **SD Card Blackbox System State Transition Logging**:
   - Define a dedicated binary log packet type (`PKT_TYPE_SYS_STATE_EVENT = 0x09` in `config.h`).
   - Push timestamped state transition records (`timestamp_us`, `prev_state`, `new_state`, `trigger_event_mask`) to the high-rate MicroSD logging pipeline whenever an FSM transition fires, enabling complete post-flight accident reconstruction and timeline synchronization.

---

## 5. Traceability & Affected Files
- Specification: `documents/notes/0020-fsmBackbonePreFlightSensorChecksAndActiveCalibrationArchitecture.md`
- Configuration: `include/config.h`
- FSM Application: `src/app/fsm.h`, `src/app/fsm.cpp`
- System Lifecycle: `src/app/app.cpp`
- Sensor Calibration Interface: `src/core/sensor/calib.h`
