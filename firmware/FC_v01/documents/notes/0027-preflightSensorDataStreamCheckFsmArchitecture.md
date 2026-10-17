---
id: "0027"
title: Pre-Flight Sensor Data Stream Health Check, Multi-Rate Latency Verification & FSM Transition Gate Architecture
author: Drone Research Team / Developer
date: 2026-10-14
time: 20:25:00 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#fsm", "#preflight", "#sensors", "#daq", "#freertos", "#safety"]
references:
  - "[[0000-firmwareStructureLayerDecision]]"
  - "[[0001-taskConfigMiddlewareIsrDecision]]"
  - "[[0002-taskEnumsAndHalAbstractionDecision]]"
  - "[[0006-taskCreationConfigEnumsAndTaskArgsPassingDecision]]"
  - "[[0008-lpfAndCalibrationTaskArchitecture]]"
  - "[[0017-bmp180TwoStepI2cTransactionAndBaroDaqArchitecture]]"
  - "[[0020-fsmBackbonePreFlightSensorChecksAndActiveCalibrationArchitecture]]"
  - "[[0022-unifiedPressureRingBufferSensorVariableConsolidationAndAttitudeTaskRemoval]]"
  - "[[0023-fsmCentricCalibrationOrchestrationAndRgbLedArchitecture]]"
  - "[[0031-sixGatePreflightTerminalDevelopmentAndTaskEncapsulationArchitecture]]"
---

# Zettel 0027: Pre-Flight Sensor Data Stream Health Check, Multi-Rate Latency Verification & FSM Transition Gate Architecture

## 1. Metadata & Scope
- **ID**: `0027`
- **Author**: Drone Research Team / Developer
- **Date**: 2026-10-14
- **Time**: 20:25:00 +07:00
- **Status**: Approved & Implemented
- **References**: `[Note 0000]`, `[Note 0002]`, `[Note 0008]`, `[Note 0017]`, `[Note 0020]`, `[Note 0022]`, `[Note 0023]`

---

## 2. Problem Statement & Motivation
In autonomous flight controller systems, allowing motors to arm when a sensor driver has failed, stalled, or produced disconnected bus artifacts is catastrophic. Prior to this implementation, the `PREFLIGHT` state in `src/app/fsm.cpp` assumed sensor streams were active and immediately attempted stationary detection and passive calibration checks:
1. **False Stationary Gating on Missing Data**: If the primary IMU (MPU6050) failed on boot or locked the I2C bus, the raw ring buffer was either empty or dormant. Reading consecutive un-updated samples yielded $\Delta \vec{a} = 0, \Delta \vec{g} = 0$, falsely satisfying stationary variance bounds.
2. **Ghost Stream Downstream Starvation**: Downstream tasks (cascaded PID loops, Kalman filters, SD loggers, and BLE telemetry) require active streaming across all 5 operational pipelines:
   - Raw IMU ($500\,\text{Hz}$)
   - Preprocessed/Filtered IMU ($500\,\text{Hz}$)
   - Raw Magnetometer ($100\,\text{Hz}$)
   - Preprocessed/Filtered Magnetometer ($100\,\text{Hz}$)
   - Barometer Pressure ($50\,\text{Hz}$)
3. **Lack of Hardware Bus Fault Indication**: If a physical sensor was disconnected or experienced an I2C transaction lockup, the drone remained indefinitely in `PREFLIGHT` with no distinct visual error latch differentiating sensor failure from motion disturbance.

---

## 3. Core Architectural Decisions

### Rule 1: First-in-Pipeline Gating Order in `handle_state_preflight()`
`sensor_data_stream_check()` is positioned strictly as **Check 1** (the entry gate) in `handle_state_preflight()`:
```cpp
if(!sensor_data_stream_check()) return;
if(!check_stationary_condition()) return;
if(!check_passive_sensor_calibration()) return;
// ...
```
Downstream stationary variance and resting calibration evaluations are completely bypassed until all 5 sensor pipelines have demonstrated continuous, active, and physically valid streaming.

### Rule 2: Multi-Rate Latency & Freshness Verification
Latency thresholds are centrally configured in `include/config.h` based on task running frequencies:
$$\Delta t_{\text{sample}} = t_{\text{now\_us}} - t_{\text{sample\_us}}$$
- **IMU Raw & Preprocessed ($500\,\text{Hz}$, period $2\,\text{ms}$)**: $\Delta t \le 50\,000\,\mu\text{s}$ (`SENSOR_STREAM_IMU_TIMEOUT_US`, allowing max 25 skipped frames).
- **Magnetometer Raw & Preprocessed ($100\,\text{Hz}$, period $10\,\text{ms}$)**: $\Delta t \le 100\,000\,\mu\text{s}$ (`SENSOR_STREAM_MAG_TIMEOUT_US`, allowing max 10 skipped frames).
- **Barometer Raw ($50\,\text{Hz}$, period $20\,\text{ms}$)**: $\Delta t \le 200\,000\,\mu\text{s}$ (`SENSOR_STREAM_BARO_TIMEOUT_US`, accommodating the periodic $1\,\text{Hz}$ temperature conversion phase).

### Rule 3: Physical Domain & Numerical Sanity Validation
Streams must not only be fresh, but also mathematically finite and within valid terrestrial physical bounds:
1. **Accelerometer Raw**: $\text{isfinite}(a_{x,y,z}) \land 0.1\,\text{g} < \|\vec{a}\| < 3.0\,\text{g}$ (rejects zero-bus disconnects and saturated readings).
2. **Gyroscope Raw**: $\text{isfinite}(g_{x,y,z}) \land \|\vec{g}\| < 200.0^\circ/\text{s}$ (rejects wild startup oscillation or bus corruption).
3. **Magnetometer Raw**: $\text{isfinite}(m_{x,y,z}) \land 0.05\,\text{G} < \|\vec{m}\| < 5.0\,\text{G}$ (guarantees sensing of ambient geomagnetic field).
4. **Barometer Pressure**: $\text{isfinite}(p) \land 30\,000\,\text{Pa} \le p \le 120\,000\,\text{Pa}$ (validates physical earth atmospheric pressure range $300..1200\,\text{hPa}$).

### Rule 4: Two-Stage Temporal Stability & Timeout Gating
1. **Healthy Window Accumulation (`SENSOR_STREAM_HEALTHY_TIME_MS = 500`)**: All 5 streams must continuously satisfy freshness and validity bounds for $500\,\text{ms}$ uninterrupted. If any sample drops or glitches, the timer resets to zero. Once $500\,\text{ms}$ elapses, `EVT_SENSOR_STREAM_OK` is asserted.
2. **Startup Grace Timeout (`SENSOR_STREAM_STARTUP_TIMEOUT_MS = 2500`)**: During initial boot, tasks require up to several hundred milliseconds to initialize hardware, take conversion delays, and fill initial ring buffers. If the streams fail to stabilize within $2.5\,\text{s}$ from entering `PREFLIGHT`, `EVT_SENSOR_STREAM_FAIL` is asserted.
3. **In-Flight Disconnect Drop Detection**: If streams were previously marked healthy (`EVT_SENSOR_STREAM_OK`) and subsequently stall while still in `PREFLIGHT`, `EVT_SENSOR_STREAM_OK` is immediately cleared and `EVT_SENSOR_STREAM_FAIL` is asserted.

### Rule 5: FSM Transition Integration & Visual Failure Latch
In `TRANSITION_RULES`:
- `Rule_Preflight_To_Ready` requires `EVT_SENSOR_STREAM_OK | EVT_STATIONARY_MET | EVT_PASSIVE_CALIB_PASS` under `MatchPolicy::ALL`.
- `EVT_SENSOR_STREAM_FAIL` is added to `blocker_mask` under `MatchPolicy::ANY`.
- When `EVT_SENSOR_STREAM_FAIL` occurs, `indicate_sensor_stream_error()` latches `LED_PATTERN_BLINK_FAST` with `PALETTE_RED`, giving unambiguous visual feedback that a hardware sensor bus or task has failed.

---

## 4. Centralized Parameters in `include/config.h`
```cpp
constexpr uint32_t SENSOR_STREAM_IMU_TIMEOUT_US     = 50000;   // 50 ms max latency for 500 Hz IMU
constexpr uint32_t SENSOR_STREAM_MAG_TIMEOUT_US     = 100000;  // 100 ms max latency for 100 Hz Mag
constexpr uint32_t SENSOR_STREAM_BARO_TIMEOUT_US    = 200000;  // 200 ms max latency for 50 Hz Baro
constexpr uint32_t SENSOR_STREAM_HEALTHY_TIME_MS    = 500;     // Continuous healthy streaming required (500 ms)
constexpr uint32_t SENSOR_STREAM_STARTUP_TIMEOUT_MS = 2500;    // Maximum time allowed for all sensor streams to be ready (2.5 s)
constexpr float    SENSOR_STREAM_BARO_MIN_PA        = 30000.0f;  // Minimum valid atmospheric pressure (300 hPa)
constexpr float    SENSOR_STREAM_BARO_MAX_PA        = 120000.0f; // Maximum valid atmospheric pressure (1200 hPa)
```

---

## 5. Traceability & Affected Files
- Specification: `documents/notes/0027-preflightSensorDataStreamCheckFsmArchitecture.md`
- Configuration: `include/config.h`
- FSM Application: `src/app/fsm.h`, `src/app/fsm.cpp`
- Tracing & Architecture Index: `documents/Firmware Structure.md`
