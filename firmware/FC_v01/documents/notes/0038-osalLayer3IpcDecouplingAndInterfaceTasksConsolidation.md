---
id: "0038"
title: Layer 3 OSAL & IPC Boundary Enforcement, Domain Struct Decoupling, and Interface Tasks Consolidation
author: Drone Research Team / Developer
date: 2026-10-26
time: 10:25:00 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#osal", "#ipc", "#middleware", "#refactoring", "#interface"]
references:
  - "[[0000-firmwareStructureLayerDecision]]"
  - "[[0001-taskConfigMiddlewareIsrDecision]]"
  - "[[0002-taskEnumsAndHalAbstractionDecision]]"
  - "[[0016-coreDirectoryCompactionAndDomainConsolidation]]"
  - "[[0023-fsmCentricCalibrationOrchestrationAndRgbLedArchitecture]]"
  - "[[0029-unificationOfInterTaskRingBufferSizeConstant]]"
---

# Zettel 0038: Layer 3 OSAL & IPC Boundary Enforcement, Domain Struct Decoupling, and Interface Tasks Consolidation

## 1. Metadata & Scope
- **ID**: `0038`
- **Author**: Drone Research Team / Developer
- **Date**: 2026-10-26
- **Status**: Approved & Active Refactoring Roadmap
- **References**: `[Note 0000]`, `[Note 0001]`, `[Note 0002]`, `[Note 0016]`, `[Note 0023]`, `[Note 0029]`

---

## 2. Problem Statement & Context
Over progressive feature development iterations (Sensors, Ble OTA, WS2812 RGB LED, Calibrations, EKF Estimators, INA226 Power, and Actuator Control), `src/middleware/ipc.h` and `src/middleware/ipc.cpp` devolved into a "kitchen sink" global dumping ground. 

This accumulation broke multiple foundational architectural rules established in Note 0000, 0001, and 0002:
1. **Target Abstraction Violations**: Inclusion of vendor-specific headers (`<esp_timer.h>`, `<NimBLEDevice.h>`) inside a Layer 3 middleware header.
2. **Layer Inversion & Cyclic Coupling**: Middleware importing Layer 2 concrete drivers (`"drivers/telemetry/ble_ota_packet.h"`, `"drivers/led/ws2812.h"`, `"drivers/rc_rx/fs_ia6b.h"`) and Layer 4 algorithms (`"core/math/matrix.h"`).
3. **Domain Pollution**: Domain-specific payload structs, FSM protocol enums, persistent calibration objects, driver pointers, and full button debounce logic residing in `ipc.h`/`ipc.cpp`.
4. **Interface Domain Fragmentation**: Dedicated WS2812 LED task runner was orphaned in `src/core/interface/led_task.cpp/.h` while `interface_tasks.h` sat as a hollow placeholder.

---

## 3. Comprehensive OSAL & IPC Entity Audit

| Declared Entity | Current Type in `ipc.h/.cpp` | True Architectural Domain | Legitimate OSAL / IPC? | Target Location | Rationale & Remediation |
|---|---|---|---|---|---|
| `i2c0_mutex`, `i2c1_mutex` | `SemaphoreHandle_t` | Layer 3 Bus Synchronization | **YES** | `src/middleware/ipc.h` | Shared hardware bus synchronization primitive protecting multi-task I2C access. |
| `spi_bus_mutex` | `SemaphoreHandle_t` | Layer 3 Bus Synchronization | **YES** | `src/middleware/ipc.h` | Protects shared SPI bus access between SD card, LoRa, and external SPI peripherals. |
| `flash_mutex` | `SemaphoreHandle_t` | Layer 3 Storage Synchronization | **YES** | `src/middleware/ipc.h` | Protects non-volatile NVS flash reads/writes across tasks. |
| `ekf_state_mutex` | `SemaphoreHandle_t` | Layer 3 State Synchronization | **YES** | `src/middleware/ipc.h` | Thread-safe guard for dual-core estimator state reads and updates. |
| `status_led_mutex` | `SemaphoreHandle_t` | Layer 3 Bus Synchronization | **NO (Redundant)** | Remove | Status LED is driven exclusively by `status_led_task` via `led_cmd_queue`. Mutex is unused and redundant. |
| `app_task_handles[]` | `TaskHandle_t[TASK_COUNT]` | Layer 3 OSAL Task Registry | **YES** | `src/middleware/ipc.h` | Central task handle registry indexed by `TaskIDs` enum for direct RTOS task notifications. |
| `init_ipc()` | `bool init_ipc()` | Layer 3 IPC Lifecycle | **YES** | `src/middleware/ipc.h` | Central boot-time allocator for mutexes, queues, and trace instrumentation. |
| Ring Buffers (`accel_raw`, `gyro_raw`, `mag_raw`, `pressure`, `power_data`, `accel`, `gyro`, `mag`, `rc_data`, `att_quat`, `att_euler`, `alt_state`) | `RingBuffer<T, 10>` | Layer 3 Inter-Task Data Pipes | **YES** | `src/middleware/ipc.h` (or topic-grouped) | Legitimate lock-free / critical section inter-task data pipes connecting DAQ, filter, estimator, and logging tasks. |
| Queues (`ota_data_queue`, `led_cmd_queue`, `calib_report_queue`, `motor_cmd_queue`) | `QueueHandle_t` | Layer 3 IPC Queues | **YES** | `src/middleware/ipc.h` | Core inter-task message pipes. Queue handles remain in IPC; payload types must be decoupled. |
| `ota_start_payload`, `ota_abort_requested`, `OtaDataChunk` | Globals & Struct | Layer 4 Telemetry / OTA | **NO** | `src/core/telemetry/ble_ota_tasks.h` | Telemetry subsystem internal state and packet buffer definitions. |
| `sd_stop_logging_requested` | `volatile bool` | Layer 4 Blackbox | **NO** | `src/core/blackbox/sd_logger_task.h` | Ad-hoc stop flag; should be signaled via FreeRTOS direct-to-task notification bit. |
| `LedPattern`, `LedCommand` | Enums & Struct | Layer 4 Interface | **NO** | `src/core/interface/interface_tasks.h` | UI display patterns and command struct coupling `PaletteColorId` (Layer 2) into Layer 3. |
| `CalibTarget`, `CalibStatus`, `CalibReportMsg` | Enums & Struct | Layer 4 Sensor Calibration | **NO** | `src/core/sensor/calib_types.h` (or `calib.h`) | Sensor calibration protocol data structures exchanged between workers and FSM coordinator. |
| `SensorCalibrationData`, `calib_data`, `calib_storage` | Struct & Globals | Layer 4 Sensor Domain Data | **NO** | `src/core/sensor/calib.h` / `calib.cpp` | Sensor calibration matrix/bias state and FlashStorageDriver pointer. Not IPC primitives. |
| `BtnEventMasks`, `ipc_pop_btn_events()`, `user_btn_isr()`, `s_btn_hold_timer` | Enums, Functions, Timer | Layer 4 Interface / User Button | **NO** | `src/core/interface/user_button.h/.cpp` | Button driver implementation, debouncing logic, and timer callbacks belonging in interface domain. |
| `PowerMeasurement` | Struct | Layer 2 Driver / Layer 4 Sensor | **NO** | `src/drivers/power/ina226.h` | Domain measurement data structure for INA226 power monitor. |
| `IndiControllerOutput`, `indi_output` | Struct & Global | Layer 4 Flight Control | **NO** | `src/core/control/control_types.h` | Flight control torque and throttle commands; only referenced in future development comments. |
| `MotorCommand`, `MotorNotificationBits` | Enums | Layer 4 Actuator | **NO** | `src/core/actuator/motor_tasks.h` | Actuator arming and emergency stop commands. |
| `FcBleServerCallbacks`, `ble_server_callbacks` | Subclass & Instance | Layer 2 BLE Driver / Layer 4 Telemetry | **NO** | `src/drivers/telemetry/ble_driver.cpp` | Concrete BLE server event callback implementation calling `NimBLEDevice` and serial logs. |

---

## 4. Architectural Decisions & Execution Specification

### Decision 1: Relocation of `led_task` to Unified `interface_tasks`
- Deleted `src/core/interface/led_task.cpp` and `src/core/interface/led_task.h`.
- Consolidated `status_led_task(void* arg)` into `src/core/interface/interface_tasks.h` and `src/core/interface/interface_tasks.cpp`.
- Deleted temporary/redundant `src/core/interface/interface_task.h` forwarder; unified exclusively on `interface_tasks.h`.
- Updated `src/app/app.cpp` to include `"core/interface/interface_tasks.h"`.

### Decision 2: Executed Layer 3 IPC Refactoring & Domain Decoupling

> **Correction 2026-11-21.** When this note was written, "Executed" over-claimed: three of the four programme items below were still open in the tree, and a 2026-11-19 audit recorded that as a defect in the ledger. The programme is genuinely executed **as of 2026-11-21** — see §6 for the trace of each remainder. Item 7 of this list is also out of date: `FcBleServerCallbacks` left `ipc.h` on 2026-11-19 ([[0053-layer3IpcAccessorSplitTaskRegistrySerializationAndHalOtaBoundary]]).
1. **Redundant Mutex Elimination**:
   - Removed `status_led_mutex` from `ipc.h` and `ipc.cpp`; status LED synchronization is serialized exclusively via `led_cmd_queue`.
2. **Interface Domain Migration**:
   - Moved `LedPattern` and `LedCommand` into `src/core/interface/interface_tasks.h`.
3. **Telemetry & OTA Migration**:
   - Moved `OtaDataChunk`, `ota_start_payload`, and `ota_abort_requested` into `src/drivers/telemetry/ble_ota_packet.h` (defined in `ble_driver.cpp`).
4. **Blackbox Logging Migration**:
   - Moved `sd_stop_logging_requested` into `src/core/blackbox/sd_logger_task.h` (defined in `sd_logger_task.cpp`).
5. **Power Driver Migration**:
   - Moved `PowerMeasurement` into `src/drivers/power/ina226.h` without creating extra header files.
6. **Calibration Domain Consolidation**:
   - Moved `SensorCalibrationData`, `calib_data`, `calib_storage`, `CalibTarget`, `CalibStatus`, and `CalibReportMsg` into `src/core/sensor/calib.h` (defined in `calib.cpp`) without creating separate `calib_types.h` or `sensor_types.h`.
7. **Preserved Middleware Entities**:
   - Retained button ISR, software timers, debouncer, and callbacks (`user_btn_isr`, `ipc_pop_btn_events`, `s_btn_hold_timer`) in `ipc.h/.cpp` per Note 0001 middleware ISR callback routing rules.
   - Retained `IndiControllerOutput` / `indi_output`, `MotorCommand` / `MotorNotificationBits` / `motor_cmd_queue`, and `FcBleServerCallbacks` / `ble_server_callbacks` in `ipc.h/.cpp`.

---

## 5. Cross-References & Traceability
- Clarifies Layer 3 boundaries established in `[[0000-firmwareStructureLayerDecision]]` and `[[0001-taskConfigMiddlewareIsrDecision]]`.
- Enforces hardware abstraction and IPC centralization rules from `[[0002-taskEnumsAndHalAbstractionDecision]]`.
- Consolidates Layer 4 `interface/` domain scoped per `[[0016-coreDirectoryCompactionAndDomainConsolidation]]`.
- Updates RGB LED worker location defined in `[[0023-fsmCentricCalibrationOrchestrationAndRgbLedArchitecture]]`.
- Aligns with unified buffer constants from `[[0029-unificationOfInterTaskRingBufferSizeConstant]]`.
- Complemented by `[[0039-onDemandBleOtaInDevelopmentStateAndDependencyInversionArchitecture]]`: Uses Dependency Inversion callback for on-demand BLE OTA task invocation.

---

## 6. Remaining Items — closed 2026-11-21

The three gaps that were still open when this note claimed "Executed":

| Original claim | What was still true in the tree | Closed by |
|---|---|---|
| Payload types decoupled from IPC | `ipc.cpp` included Layer 4 headers (`interface_tasks.h`, `calib.h`) only to size `LedCommand` / `CalibReportMsg` | `include/ipc_types.h` — [[0053-layer3IpcAccessorSplitTaskRegistrySerializationAndHalOtaBoundary]] |
| Controlled task registry | `ble_ota_tasks.cpp` wrote `app_task_handles[]` directly at 6 sites, racing Layer 5's suspension sweep | `ipc_register_task()` / `ipc_get_task()` — [[0053-layer3IpcAccessorSplitTaskRegistrySerializationAndHalOtaBoundary]] |
| Hardware behind HAL | `esp_ota_*` / `esp_partition_*` called directly from Layer 4 | `HAL_OTA` + `platforms/targets/espressif/ota.cpp` — [[0053-layer3IpcAccessorSplitTaskRegistrySerializationAndHalOtaBoundary]] |

**Verification status:** none of the three has been compiled. The build gate (both `fc-v01` environments) and the OTA smoke test are still owed before this note can be read as *verified* rather than *executed*.

- Amends: [[0002-taskEnumsAndHalAbstractionDecision]] Rule 6 — the blanket "everything through `ipc.h`" wording is replaced by ownership / types / access.
- Context of the commit: [[0054-baselineCommitWorktreeRetirementAndOnedriveGitHazard]].
