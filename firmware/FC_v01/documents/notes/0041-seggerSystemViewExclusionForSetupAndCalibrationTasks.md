---
id: "0041"
title: SEGGER SystemView Tracing Scope & Exclusion of Maintenance and Calibration Tasks
author: Drone Research Team / Developer
date: 2026-10-27
time: 22:34:00 +07:00
tags: ["#zettelkasten", "#firmware", "#profiling", "#fc_v01", "#sysview", "#openocd", "#freertos", "#wcet", "#calibration"]
references:
  - "[[0006-taskCreationConfigEnumsAndTaskArgsPassingDecision]]"
  - "[[0007-systemViewTracingAndRTOSTimingAnalysisConventions]]"
  - "[[0025-rotEllipsoidMagCalibSimdAdamEkfUnitSphereStackLifecycle]]"
  - "[[0026-sixPointAccelCalibSimdEllipsoidFitFsmLedArchitecture]]"
  - "[[0040-fsmCalibrationTerminalStateAndRebootBoundaryDecision]]"
---

# Zettel 0041: SEGGER SystemView Tracing Scope & Exclusion of Maintenance and Calibration Tasks

## 1. Metadata & Scope
- **ID**: `0041`
- **Author**: Drone Research Team / Developer
- **Date**: 2026-10-27
- **Time**: 22:34:00 +07:00
- **Status**: Approved & Enforced Architectural Decision
- **Module Context**: Layer 3 Middleware (`src/middleware/sysview_tracing.h/.cpp`), Layer 4 Calibration Tasks (`src/core/sensor/calib_tasks.cpp`), RTOS Timing Analyzers (`analyzers/sysview_dual_core_analyzer.py`)

---

## 2. Core Architectural Principle: In-Flight Tracing Scope
SEGGER SystemView trace instrumentation in `FC_v01` is strictly architected for **in-flight real-time CPU profiling, scheduling latency validation, and execution jitter verification**.

The SystemView tracing infrastructure exists specifically to verify:
1. **500 Hz IMU Real-Time Deadline Compliance**: Ensuring `accel_gyro_daq_task` and `sensor_preprocess_task` execute within their strict 2,000 $\mu\text{s}$ period with sub-microsecond jitter.
2. **Cascaded Control & Estimation Latency**: Profiling `ekf_roll_pitch_task`, `ekf_yaw_task`, `ekf_altitude_task`, and motor output mixing to verify total algorithmic delay from gyro sample to PWM output.
3. **Core Affinity & Priority Preemption Determinism**: Verifying Core 0 vs. Core 1 load balancing and non-blocking inter-task communication over lock-free static ring buffers.

---

## 3. Rationale for Excluding Calibration Tasks

Interactive setup and sensor calibration routines (`task_calib_gyro`, `task_calib_baro`, `task_calib_accel`, `task_calib_mag`) were previously instrumented with `SYSVIEW_START` and `SYSVIEW_END` markers. This practice is formally prohibited due to three architectural issues:

### 3.1 Severe Trace Metric Distortion
Calibration tasks are inherently non-flight, asynchronous setup operations. For instance:
- `task_calib_accel` waits interactively for the pilot to position the drone across 6 physical orientations, holding stationary for 2.0 seconds per orientation.
- `task_calib_mag` waits for manual tumble rotations and user button clicks.
- Initial settling delays (`vTaskDelay(CALIB_SETTLING_DELAY_MS)`) hold tasks for up to 1,500 ms.

Instrumenting these phases with SystemView markers introduces synthetic Worst-Case Execution Times (WCET) on the order of hundreds of milliseconds or seconds, completely corrupting RTOS automated timing analysis and jitter metrics.

### 3.2 JTAG Apptrace Buffer Saturation
SystemView events are streamed over the on-chip USB-JTAG interface (`esp-builtin`) via a finite circular memory buffer configured with `CONFIG_SYSVIEW_BUF_WAIT_TMO=0`. Emitting high-volume SystemView markers during multi-epoch numerical solvers (such as the SIMD Adam ellipsoid fit running hundreds of iterations) floods the JTAG buffer with non-operational trace packets, risking buffer overflow drops when transitioning into operational states.

### 3.3 Separation of Concerns (Bench Setup vs. In-Flight Execution)
SystemView profiling is an engineering tool for verifying hard real-time flight deadlines under full operational task load. Bench setup, sensor offset calibration, and flash persistence occur strictly on the ground prior to arming (`STATE_CALIB`). Instrumenting maintenance routines violates the principle that SystemView markers should reflect repeatable periodic flight loops.

---

## 4. Architectural Rules & Implementation Policy
1. **Zero SystemView Instrumentation in `src/core/sensor/calib_tasks.cpp`**: All calibration tasks must omit `SYSVIEW_START` and `SYSVIEW_END` macros.
2. **Permitted SystemView Targets**: Only continuous, periodic operational tasks executing in `STATE_PREFLIGHT`, `STATE_READY`, `STATE_TAKEOFF`, and `STATE_FLIGHT` may contain SystemView markers (`SYSVIEW_START` / `SYSVIEW_END`):
   - `TASK_ACCEL_GYRO_DAQ_ID` (500 Hz)
   - `TASK_SENSOR_PREPROCESS_ID` (500 Hz)
   - `TASK_MAG_DAQ_ID` (75 Hz)
   - `TASK_BARO_DAQ_ID` (50 Hz)
   - `TASK_POWER_DAQ_ID` (10 Hz)
   - `TASK_EKF_ATTITUDE_ID` (100 Hz)
   - `TASK_EKF_YAW_ID` (100 Hz)
   - `TASK_EKF_ALTITUDE_ID` (100 Hz)
   - `TASK_MOTOR_ID` (500 Hz)
   - `TASK_SD_LOGGER_ID` (100 Hz)
   - `TASK_BLE_TELEM_ID` (20 Hz)
   - `TASK_RC_RECEIVER_ID` (50 Hz)
   - `TASK_FSM_ID` (100 Hz)
