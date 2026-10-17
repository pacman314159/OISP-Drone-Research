---
id: "0029"
title: Unification of Inter-Task Ring Buffer Capacity Constant
author: Drone Research Team / Developer
date: 2026-10-15
time: 18:40:00 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#freertos", "#ipc", "#ringbuffer"]
references:
  - "[[0000-firmwareStructureLayerDecision]]"
  - "[[0001-taskConfigMiddlewareIsrDecision]]"
  - "[[0002-taskEnumsAndHalAbstractionDecision]]"
  - "[[0008-lpfAndCalibrationTaskArchitecture]]"
  - "[[0017-bmp180TwoStepI2cTransactionAndBaroDaqArchitecture]]"
  - "[[0022-unifiedPressureRingBufferSensorVariableConsolidationAndAttitudeTaskRemoval]]"
  - "[[0028-ina226PowerModuleI2c1DaqAndBatteryMonitoringArchitecture]]"
---

# Zettel 0029: Unification of Inter-Task Ring Buffer Capacity Constant

## 1. Metadata & Scope
- **ID**: `0029`
- **Author**: Drone Research Team / Developer
- **Date**: 2026-10-15
- **Time**: 18:40:00 +07:00
- **Status**: Approved & Implemented
- **References**: `[Note 0000]`, `[Note 0001]`, `[Note 0002]`, `[Note 0008]`, `[Note 0017]`, `[Note 0022]`, `[Note 0028]`

---

## 2. Problem Statement & Architecture Tensions

1. **Semantic Misnomer**:
   Historically, `IMU_RAW_RING_BUF_SIZE = 10` was introduced for raw accelerometer and gyroscope streams. As additional sensors were integrated (magnetometer in Note 0008, barometer in Note 0017/0022, and INA226 power monitoring in Note 0028), their raw buffers and timestamp queues reused `IMU_RAW_RING_BUF_SIZE`. This created a semantic inaccuracy, as Barometer (`pressure`, `pressure_timestamp`) and Power (`power_data`, `power_timestamp`) are not IMU sensors.
2. **Redundant Duplicate Constants**:
   Both `IMU_RAW_RING_BUF_SIZE` and `IMU_FILT_RING_BUF_SIZE` were statically defined to the identical value (`10`). Maintaining separate identifiers provided no architectural value and introduced boilerplate in snapshot array declarations (e.g. duplicating local array sizing logic in `sd_logger_task.cpp`).
3. **Cognitive & Maintenance Overhead**:
   Developers adding new sensor streams or logging channels had to choose between raw and filt constants for sizing non-IMU queues, risking divergence or inconsistent usage.

---

## 3. Core Architectural Decisions & Specifications

### Rule 1: Single Unified Ring Buffer Size Constant (`RING_BUF_SIZE`)
In `include/config.h`, the dual constants `IMU_RAW_RING_BUF_SIZE` and `IMU_FILT_RING_BUF_SIZE` are replaced with a single canonical capacity constant:
```cpp
// include/config.h
constexpr int RING_BUF_MAX_SIZE = 100; // Maximum allowable ring buffer size
constexpr int RING_BUF_SIZE     = 10;  // Unified inter-task ring buffer capacity
```

### Rule 2: Uniform Buffer Sizing Across Layer 3 Middleware IPC
All thread-safe inter-task ring buffers declared in `src/middleware/ipc.h` and instantiated in `src/middleware/ipc.cpp` uniformly instantiate `RingBuffer<T, RING_BUF_SIZE>`:
- **Raw Sensor Streams**: `accel_raw`, `gyro_raw`, `mag_raw`, `pressure`, `power_data`
- **Timestamp Queues**: `raw_accel_gyro_timestamp`, `raw_mag_timestamp`, `pressure_timestamp`, `power_timestamp`
- **Filtered Sensor Streams**: `accel`, `gyro`, `mag`
- **Filtered Timestamp Queues**: `filtered_accel_gyro_timestamp`, `filtered_mag_timestamp`

### Rule 3: Uniform Consumer Snapshot Array Sizing
Downstream consumer tasks extracting snapshots via `RingBuffer::get_snapshot()` size stack buffers uniformly with `RING_BUF_SIZE` (e.g. in `src/core/blackbox/sd_logger_task.cpp`):
```cpp
uint32_t ts_snap[RING_BUF_SIZE];
Vec3<float> gyro_snap[RING_BUF_SIZE];
Vec3<float> accel_snap[RING_BUF_SIZE];
```

---

## 4. Cross-References & Traceability
- [[0001-taskConfigMiddlewareIsrDecision]]: Centralized RTOS configuration in `include/config.h`.
- [[0008-lpfAndCalibrationTaskArchitecture]]: Filtered sensor ring buffer outputs.
- [[0017-bmp180TwoStepI2cTransactionAndBaroDaqArchitecture]]: Barometer pressure ring buffer injection.
- [[0022-unifiedPressureRingBufferSensorVariableConsolidationAndAttitudeTaskRemoval]]: Unified pressure ring buffer and filtered variable streamlining.
- [[0028-ina226PowerModuleI2c1DaqAndBatteryMonitoringArchitecture]]: INA226 power measurement ring buffer.
