---
id: "0031"
title: Six-Gate Pre-Flight Safety Pipeline, Terminal Development Mode, Subsystem Task Encapsulation & Battery Enforcement Architecture
author: Drone Research Team / Developer
date: 2026-10-16
time: 19:48:00 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#fsm", "#preflight", "#development", "#safety", "#freertos", "#encapsulation"]
references:
  - "[[0000-firmwareStructureLayerDecision]]"
  - "[[0001-taskConfigMiddlewareIsrDecision]]"
  - "[[0002-taskEnumsAndHalAbstractionDecision]]"
  - "[[0006-taskCreationConfigEnumsAndTaskArgsPassingDecision]]"
  - "[[0013-sdCardSpiStorageAndFlightLoggingArchitecture]]"
  - "[[0018-sdCardLoggingRestorationAndUserButtonShutdown]]"
  - "[[0020-fsmBackbonePreFlightSensorChecksAndActiveCalibrationArchitecture]]"
  - "[[0023-fsmCentricCalibrationOrchestrationAndRgbLedArchitecture]]"
  - "[[0024-internalFlashNvsStorageHalAndSensorCalibrationPersistence]]"
  - "[[0027-preflightSensorDataStreamCheckFsmArchitecture]]"
  - "[[0028-ina226PowerModuleI2c1DaqAndBatteryMonitoringArchitecture]]"
  - "[[0030-rmtHardwarePpmDecodingFsIa6bAndBleTelemetryArchitecture]]"
  - "[[0032-rcReceiverCommandBlackboxSdCardLoggingArchitecture]]"
---

# Zettel 0031: Six-Gate Pre-Flight Safety Pipeline, Terminal Development Mode, Subsystem Task Encapsulation & Battery Enforcement Architecture

## 1. Metadata & Scope
- **ID**: `0031`
- **Author**: Drone Research Team / Developer
- **Date**: 2026-10-16
- **Time**: 19:48:00 +07:00
- **Status**: Approved & Implemented
- **References**: `[Note 0000]`, `[Note 0001]`, `[Note 0002]`, `[Note 0006]`, `[Note 0013]`, `[Note 0018]`, `[Note 0020]`, `[Note 0023]`, `[Note 0024]`, `[Note 0027]`, `[Note 0028]`, `[Note 0030]`

---

## 2. Problem Statement & Motivation

As the flight controller grew to support multi-rate DAQ pipelines, NVS flash calibration, MicroSD blackbox logging, INA226 power monitoring, and hardware RMT PPM decoding, three major architectural liabilities emerged:

1. **Monolithic Task Setup & Leaky Abstractions in `app.cpp`**:
   - `app_start()` had accumulated domain-specific logic including NVS calibration parameter loading, MicroSD file indexing, binary header formatting, and hardware detection logging.
   - Task runners depended on external pre-initialization by `app_start()`, violating the single-responsibility principle and cluttering board bringup.
2. **Race Conditions During FSM Supervision Startup**:
   - The System State Machine (`fsm_task`) was initialized and spawned *before* subsequent worker tasks (telemetry, storage, power, RC receiver). The supervisor began evaluating system events while sensor tasks and ring buffers were still unallocated or unstarted.
3. **Fragile Pre-Flight Failover & Unbounded State Oscillations**:
   - The pre-flight check suite had arbitrary retry behavior. If a safety check failed, the system could intermittently drop into a debug state and attempt uncoordinated re-entry, risking arming on compromised sensors or corrupted state variables.
   - Global or file-scope static variables tracking pre-flight timers across repeated entries risked stale state carryover.
   - A bypass macro (`REQUIRE_BATTERY_FOR_ARMING = false`) existed for bench debugging, introducing the critical danger of deploying firmware that permits motor arming without battery voltage verification.

---

## 3. The 6-Gate Pre-Flight Safety Pipeline

Flight readiness is determined through a strictly sequential, non-blocking 6-Gate pipeline inside `handle_state_preflight()` (`src/app/fsm.cpp`):

```
+----------------------------------------------------------------------------------------------------+
|                                    BOOT (UNINIT -> PREFLIGHT)                                      |
+----------------------------------------------------------------------------------------------------+
                                                  |
                                                  v
+-----------------------+     Fail / Timeout      +--------------------------------------------------+
|  Gate 1: Stream Check | ----------------------> |                                                  |
+-----------------------+                         |                                                  |
           | Pass                                 |                                                  |
           v                                      |                                                  |
+-----------------------+     Fail / Timeout      |                                                  |
| Gate 2: Stationary    | ----------------------> |                                                  |
+-----------------------+                         |                                                  |
           | Pass                                 |                                                  |
           v                                      |                                                  |
+-----------------------+     Fail / Incomplete   |                                                  |
| Gate 3: Passive Calib | ----------------------> |            STATE_DEVELOPMENT                     |
+-----------------------+                         |            (Terminal State)                      |
           | Pass                                 |            - Zero Throttle (PWM = 0)             |
           v                                      |            - Solid Amber LED Alert               |
+-----------------------+     Incline > 10 deg    |            - Permanently Locked Until Hardware    |
| Gate 4: Tilt Check    | ----------------------> |              Power-Cycle / Cold Reboot           |
+-----------------------+                         |                                                  |
           | Pass                                 |                                                  |
           v                                      |                                                  |
+-----------------------+     V < 3.3V/cell / Bad |                                                  |
| Gate 5: Power Check   | ----------------------> |                                                  |
+-----------------------+                         |                                                  |
           | Pass                                 |                                                  |
           v                                      |                                                  |
+-----------------------+     Failsafe / No Link  |                                                  |
| Gate 6: RC Receiver   | ----------------------> |                                                  |
+-----------------------+                         +--------------------------------------------------+
           | Pass
           v
+-----------------------+
|    STATE_READY        | ---> Ready for Pilot Arming (Green LED)
+-----------------------+
```

### The 6 Gates:
1. **Gate 1 (`sensor_data_stream_check`)**:
   - Evaluates buffer occupancy across all 5 pipelines (IMU raw, IMU filt, Mag raw, Mag filt, Baro).
   - Validates multi-rate stream freshness against `SENSOR_STREAM_*_TIMEOUT_US`.
   - Asserts physical sanity bounds: accel norm $\in [0.5, 2.0]\,g$, mag norm $\in [0.1, 1.5]\,\text{Gauss}$, baro pressure $\in [300, 1200]\,\text{hPa}$.
2. **Gate 2 (`check_stationary_condition`)**:
   - Enforces quiescent vehicle stability on the launch pad.
   - Requires continuous resting state ($\Delta \vec{g} < 0.5^\circ/\text{s}$, $\Delta \vec{a} < 0.05\,g$) for $\ge 1500\,\text{ms}$ (`STATIONARY_TIME_REQUIRED_MS`).
3. **Gate 3 (`check_passive_sensor_calibration`)**:
   - Verifies that preprocessed sensor streams possess valid resting calibration parameters.
   - Evaluates filtered gyro bias $\le 0.05\,\text{rad/s}$ and filtered accel gravity alignment $\approx 1.0\,g \pm 0.1\,g$.
4. **Gate 4 (`check_platform_tilt`)**:
   - Computes platform tilt angle $\theta = \arccos(a_z / \|\vec{a}\|) \times (180 / \pi)$.
   - Rejects arming if ground inclination exceeds $10.0^\circ$ (`PREFLIGHT_MAX_TILT_DEG`).
5. **Gate 5 (`check_power_initial_condition`) & Mandatory Battery Enforcement**:
   - Reads `power_data` ring buffer published by `power_daq_task`.
   - Rejects invalid readings ($V < 1000\,\text{mV}$ or $V > 36000\,\text{mV}$, non-finite current/power).
   - Dynamically estimates cell count $N = \text{round}(V_{\text{pack}} / 3.8\,\text{V})$ (clamped $1\text{S}\text{--}8\text{S}$).
   - **Absolute Removal of `REQUIRE_BATTERY_FOR_ARMING`**: The bench mode bypass is entirely eliminated. If per-cell voltage $V_{\text{cell}} < 3.3\,\text{V}$ (`BATTERY_CELL_MIN_VOLTAGE_MV = 3300.0f`), arming is strictly refused and fails immediately.
   - **Startup Timeout Failover**: If no battery is connected or `power_data` remains empty after `SENSOR_STREAM_STARTUP_TIMEOUT_MS` (2.5 s), Gate 5 asserts `EVT_BATTERY_LOW` and transitions immediately to terminal `STATE_DEVELOPMENT`.
6. **Gate 6 (`check_rc_controller_signal`)**:
   - Validates that `rc_data` is actively streaming from RMT PPM hardware decoding.
   - Enforces `ch.failsafe == false` and verifies all pilot command channels are within valid microsecond PWM frames ($900\text{--}2100\,\mu\text{s}$).
   - **Startup Timeout Failover**: If no RC stream arrives within `SENSOR_STREAM_STARTUP_TIMEOUT_MS` (2.5 s) or failsafe is asserted, Gate 6 asserts `EVT_RC_SIGNAL_FAIL` and transitions immediately to `STATE_DEVELOPMENT`.

---

## 4. Terminal `DEVELOPMENT` State Architecture

### A. One-Way Failover & Immutable Lock
`PREFLIGHT` is accessed **strictly once on cold boot** (`UNINIT -> PREFLIGHT`).
If any of the 6 pre-flight gates fail or exceed their timeout period:
- The FSM transitions immediately to `STATE_DEVELOPMENT`.
- Motors are locked to absolute zero output (`PWM = 0`).
- The status LED latches to solid amber (`RGB_LED_COLOR_AMBER`).
- **No Automatic Retry / Recovery**: The system remains locked in `STATE_DEVELOPMENT` indefinitely. No periodic state recovery, automatic event clearing, or speculative retry is permitted. The developer or pilot must correct the hardware fault and execute a physical power-cycle or reset.

### B. Function-Local Gate Encapsulation
Because `PREFLIGHT` executes only once from boot, all gate tracking variables (e.g., `stream_check_start_ms`, `healthy_start_ms`, `last_raw_accel`, `stationary_start_ms`) are declared as **function-local static variables** directly inside their respective check functions (`sensor_data_stream_check()`, `check_stationary_condition()`, etc.):
- Eliminates global/file-scope static variable leakage in `fsm.cpp`.
- Prevents cross-gate state contamination.
- Removes complex multi-pass reset state machines.

### C. Development Graceful Shutdown (`DEVELOPMENT -> POSTFLIGHT`)
To facilitate bench diagnostics without risking filesystem corruption:
- While motor arming is permanently locked in `DEVELOPMENT` (preventing hazardous spin-up), holding the user button for $\ge 3000\,\text{ms}$ (`USER_BTN_HOLD_DURATION_MS`) triggers `Rule_Development_To_Postflight` via `EVT_BTN_LONG_HOLD` (short clicks are ignored).
- This transitions the FSM from **`DEVELOPMENT` directly to `POSTFLIGHT`**, cleanly triggering `sd_stop_logging_requested = true;` and notifying `sd_logger_task` to flush all buffered sectors, sync POSIX metadata, and close the file before the developer disconnects USB or removes the MicroSD card.

---

## 5. Subsystem Task Encapsulation & `app.cpp` Decoupling

To enforce the 5-layer architecture and maintain a clean, declarative `app_start()`, subsystem lifecycle responsibilities are offloaded directly to their owning Layer 4 task runners:

### A. NVS Calibration Loading Offloaded to `sensor_preprocess_tasks`
- **Previous Design**: `app_start()` called `calib_init(&flash_driver)`, loading persistent calibration offsets and scaling into Layer 3 IPC before spawning tasks.
- **New Architecture**: `accel_gyro_preprocess_task` directly invokes `calib_init(&flash_driver)` during its own startup preamble before entering its 500 Hz flight loop.
- **Fail-Safe Ingestion Guard**: If calibration parameters are missing or corrupted in NVS, the preprocessors (`accel_gyro_preprocess_task` and `mag_preprocess_task`) enter a dormant state: they refuse to push uncalibrated samples into downstream filtered ring buffers (`accel`, `gyro`, `mag`). Gate 1 (`sensor_data_stream_check`) detects the starving buffer and safely transitions the system to `STATE_DEVELOPMENT`.

### B. MicroSD Logger Bootstrapping Offloaded to `sd_logger_task`
- **Previous Design**: `app_start()` executed `sd_driver.is_open()`, discovered filenames via `sd_driver.open_next_log_file()`, and formatted the binary flight header.
- **New Architecture**: All filesystem verification, session ID resolution, file opening, and binary header writing are encapsulated inside `sd_logger_task` during task startup.
- `app.cpp` retains only the compile-time task creation guard:
  ```cpp
  if constexpr(ACTIVE_SD_LOG_PACKETS_MASK != LOG_MASK_NONE){
    xTaskCreatePinnedToCore(sd_logger_task, TASK_SD_LOGGER_NAME, ...);
  }
  ```

### C. INA226 Power Module Logging Offloaded to `power_daq_task`
- Hardware bus initialization (`i2c1.init()`) and sensor detection (`ina.init()`) remain visible in `app.cpp` to ensure hardware bringup traceability.
- Operational status logging is encapsulated inside `power_daq_task`, querying `ina->is_initialized()` to log runtime health without polluting board setup.

### D. Deferral of System State Machine (`fsm_init()`)
- `fsm_init()` and the creation of `fsm_task` are moved to the **very end of `app_start()`**.
- This guarantees that all driver singletons, synchronization mutexes, ring buffers, DAQ tasks, and preprocessors are fully instantiated and running before state supervision begins.

---

## 6. Structural Summary Table

| Domain / Subsystem | Previous Implementation | New Standard Architecture |
|---|---|---|
| **Pre-Flight Evaluation** | Ad-hoc checks with loop re-entry | Strict 6-Gate sequential pipeline in `handle_state_preflight()` |
| **Failover Action** | Transient debug warnings / retry | Immediate one-way transition to terminal `STATE_DEVELOPMENT` |
| **Development Mode** | Temporary state with potential transitions | Terminal lock: zero throttle, amber LED, requires hard reboot |
| **Battery Arming Policy** | Configurable via `REQUIRE_BATTERY_FOR_ARMING` | Strictly enforced: macro removed, $V_{\text{cell}} \ge 3.3\,\text{V}$ mandatory |
| **NVS Calibration Init** | Executed centrally in `app_start()` | Encapsulated in `accel_gyro_preprocess_task` preamble |
| **MicroSD File Setup** | Checked and formatted in `app_start()` | Encapsulated in `sd_logger_task` startup |
| **Power Module Logging** | Verbose prints in `app_start()` | Encapsulated in `power_daq_task` via `ina->is_initialized()` |
| **FSM Lifecycle** | Spawned midway through `app_start()` | Spawned strictly at the end of `app_start()` after all workers |
