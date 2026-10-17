---
id: "0022"
title: Unified Pressure Ring Buffer, Filtered Sensor Variable Consolidation & Phantom Attitude Task Removal
author: Drone Research Team / Developer
date: 2026-10-11
time: 16:15:00 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#freertos", "#ipc", "#sensors"]
references:
  - "[[0000-firmwareStructureLayerDecision]]"
  - "[[0001-taskConfigMiddlewareIsrDecision]]"
  - "[[0002-taskEnumsAndHalAbstractionDecision]]"
  - "[[0008-lpfAndCalibrationTaskArchitecture]]"
  - "[[0016-coreDirectoryCompactionAndDomainConsolidation]]"
  - "[[0017-bmp180TwoStepI2cTransactionAndBaroDaqArchitecture]]"
  - "[[0021-bleTelemetryDispatcherConsolidationAndSubscriptionGating]]"
  - "[[0029-unificationOfInterTaskRingBufferSizeConstant]]"
---

# Zettel 0022: Unified Pressure Ring Buffer, Filtered Sensor Variable Consolidation & Phantom Attitude Task Removal

## 1. Metadata & Scope
- **ID**: `0022`
- **Author**: Drone Research Team / Developer
- **Date**: 2026-10-11
- **Time**: 16:15:00 +07:00
- **Status**: Approved & Implemented
- **References**: `[Note 0000]`, `[Note 0001]`, `[Note 0002]`, `[Note 0008]`, `[Note 0016]`, `[Note 0017]`, `[Note 0021]`

---

## 2. Problem Statement & Architecture Tensions

1. **Phantom Attitude Task Ambiguity**:
   `include/config.h` previously defined `TASK_ATTITUDE_NAME`, `TASK_ATTITUDE_ID`, `TASK_ATTITUDE_CORE`, `TASK_ATTITUDE_PRIORITY`, `TASK_ATTITUDE_FREQ_HZ`, and `TASK_ATTITUDE_STACK_SIZE`, and `sysview_tracing.cpp` registered a SystemView resource for it. However, no standalone attitude task existed in `src/app/` or `src/core/control/` (flight control algorithms are implemented as cascaded rate loops in `control_tasks.cpp`). This caused confusion and scheduler ambiguity regarding actual task count and RTOS allocations.
2. **Orphan FreeRTOS Queue (`gyro_telemetry_queue`)**:
   A legacy `QueueHandle_t gyro_telemetry_queue` (allocating 60 $\times$ 16B in SRAM) was retained in `src/middleware/ipc.h` and initialized in `ipc_init()`, but never written or read. Telemetry dispatching migrated entirely to lock-free ring buffers (`ble_telem_tasks.cpp` via `BleTelemetryChannel`).
3. **Dead Single-Instance Global Variables**:
   `ipc.h` declared global variables (`Vec3<float> accel`, `gyro`, `mag`, `uint32_t accel_gyro_timestamp`, `mag_timestamp`) that were populated in `sensor_preprocess_tasks.cpp` inside critical sections, but never read by downstream consumers (`fsm.cpp`, `sd_logger_task.cpp`, `ble_telem_tasks.cpp`), which read directly from ring buffers. These also caused naming collisions when streamlining ring buffer identifiers.
4. **Redundant Barometer LPF Ring Buffers**:
   Analysis of BMP180 50 Hz barometric data demonstrated that discrete low-pass filtering on pressure is unnecessary and adds phase delay. Maintaining distinct `pressure_raw` and `pressure_filtered` ring buffers was redundant, with `pressure_filtered` remaining an unwritten ghost buffer.

---

## 3. Core Architectural Decisions & Specifications

### Rule 1: Deprecation and Removal of Phantom Attitude Task
All declarations of `TASK_ATTITUDE_*` are removed from `include/config.h` and `sysview_tracing.cpp`:
- `TaskIDs`, `TaskCores`, `TaskPriorities`, `TaskFrequenciesHz`, and `TaskSizesBytes` reflect only the actual FreeRTOS tasks (`TASK_ACCEL_GYRO_DAQ`, `TASK_ACCEL_GYRO_PREPROCESS`, `TASK_BARO_DAQ`, `TASK_MAG_DAQ`, `TASK_MAG_PREPROCESS`, `TASK_BLE_TELEM`, `TASK_SD_LOGGER`, `TASK_BLE_OTA`, `TASK_FSM`, etc.).
- Slot allocations in `app_task_handles[TASK_COUNT]` accurately reflect the running system tasks.

### Rule 2: Removal of Orphan IPC Primitives (`gyro_telemetry_queue` & Scalar Globals)
- `gyro_telemetry_queue` is completely excised from `src/middleware/ipc.h` and `src/middleware/ipc.cpp`.
- The unused global scalar variables `accel`, `gyro`, `mag`, `accel_gyro_timestamp`, and `mag_timestamp` are deleted.
- Redundant critical-section writes in `src/core/sensor/sensor_preprocess_tasks.cpp` are eliminated, relying entirely on thread-safe `RingBuffer::push()` atomic mechanics.

### Rule 3: Harmonized Ring Buffer Naming (`accel`, `gyro`, `mag`)
Filtered inter-task ring buffers in `src/middleware/ipc.h` are renamed to clean, canonical identifiers:
```cpp
extern RingBuffer<Vec3<float>, IMU_FILT_RING_BUF_SIZE> accel;
extern RingBuffer<Vec3<float>, IMU_FILT_RING_BUF_SIZE> gyro;
extern RingBuffer<Vec3<float>, IMU_FILT_RING_BUF_SIZE> mag;
```
All consumers (`fsm.cpp`, `sd_logger_task.cpp`, `ble_telem_tasks.cpp`) and producers (`sensor_preprocess_tasks.cpp`) access these canonical buffers directly.

### Rule 4: Unification of Barometer Pressure Ring Buffer
`pressure_raw` and `pressure_filtered` are unified into a single ring buffer, with a matching timestamp ring buffer:
```cpp
extern RingBuffer<float, IMU_RAW_RING_BUF_SIZE>    pressure;
extern RingBuffer<uint32_t, IMU_RAW_RING_BUF_SIZE> pressure_timestamp;
```
- Produced by `baro_daq_task` in `src/core/sensor/daq_tasks.cpp`.
- Consumed by `sd_logger_task.cpp` and `ble_telem_tasks.cpp`.

---

## 4. Cross-References & Traceability
- [[0001-taskConfigMiddlewareIsrDecision]]: Centralized IPC mutex and queue initialization.
- [[0002-taskEnumsAndHalAbstractionDecision]]: Centralized task enum configurations in `include/config.h`.
- [[0008-lpfAndCalibrationTaskArchitecture]]: Layer 4 LPF filter execution and ring buffer outputs.
- [[0017-bmp180TwoStepI2cTransactionAndBaroDaqArchitecture]]: BMP180 50 Hz DAQ loop and unified pressure buffer injection.
- [[0021-bleTelemetryDispatcherConsolidationAndSubscriptionGating]]: Unified BLE telemetry dispatcher reading from ring buffers.
- [[0029-unificationOfInterTaskRingBufferSizeConstant]]: Unification of IMU_RAW_RING_BUF_SIZE and IMU_FILT_RING_BUF_SIZE into RING_BUF_SIZE.
- [[0032-dualCoreEkfStateEstimationAttitudeRingBufferArchitecture]]: Dual-core 7-state quaternion EKF attitude estimator replacing ghost task.
