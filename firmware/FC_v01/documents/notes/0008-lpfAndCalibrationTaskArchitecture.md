---
id: "0008"
title: Layer 4 LPF Filter & Calibration Task Architecture Specification
author: Drone Research Team / Developer
date: 2026-09-30
time: 13:46:00 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#lpf", "#simd", "#freertos", "#dsp"]
references:
  - "[[0000-firmwareStructureLayerDecision]]"
  - "[[0001-taskConfigMiddlewareIsrDecision]]"
  - "[[0002-taskEnumsAndHalAbstractionDecision]]"
  - "[[0003-simdHalArchitectureAndLayeringRule]]"
  - "[[0006-taskCreationConfigEnumsAndTaskArgsPassingDecision]]"
---

# Zettel 0008: Layer 4 LPF Filter & Calibration Task Architecture Specification

## 1. Metadata & Context
- **ID**: `0008`
- **Author**: Drone Research Team / Developer
- **Date**: 2026-09-30
- **Time**: 13:46:00 +07:00
- **Status**: Approved & Enforced
- **References**: `[Note 0000]`, `[Note 0003]`, `[Note 0006]`

---

## 2. Core Architectural Decisions

### Rule 1: Layer 4 Submodule Scoping (`src/core/filters/` & `src/core/calib/`)
- Digital signal processing (DSP) filters and sensor calibration routines belong exclusively to **Layer 4 (Flight Core & Algorithms Layer)**.
- `src/core/filters/` houses discrete low-pass filter algorithms (`lpf.h`) and task runners (`filter_tasks.h`, `filter_tasks.cpp`).
- `src/core/calib/` houses sensor bias removal, axis alignment, scaling matrices, and hard/soft-iron magnetometer compensation routines.

### Rule 2: Tustin Bilinear 1st-Order Low-Pass Filter State (`LowPassFilter1stOrder`)
- Filters use discrete 1st-order difference equations derived from s-domain $H(s) = \frac{\omega_c}{s + \omega_c}$ via the Tustin bilinear transform ($s = \frac{2}{T} \cdot \frac{z - 1}{z + 1}$):
  $$y[i] = A \cdot (x[i] + x[i-1]) - B \cdot y[i-1]$$
  where $A = \frac{T \cdot \omega_c}{T \cdot \omega_c + 2}$, $B = \frac{T \cdot \omega_c - 2}{T \cdot \omega_c + 2}$.
- State structures are named using `PascalCase` (`struct LowPassFilter1stOrder`), containing state vectors $x_{prev}, y_{prev}$ and coefficients $a, b$.
- State instances are declared task-locally to prevent state cross-contamination across sensor channels.

### Rule 3: Dual Execution Engine (`ENABLE_SIMD_ACCELERATION`)
- Pure function `lpf_apply(LowPassFilter1stOrder* filter, const Vec3<float>& x_curr)` implements dual execution paths:
  1. `#if (!ENABLE_SIMD_ACCELERATION)`: Portable C++ scalar calculation loop.
  2. `#else`: Hardware-accelerated 3D vector operations via `hal_simd::add`, `hal_simd::scale`, `hal_simd::sub`.

### Rule 4: IPC Clean Variable Naming & Critical Section Updates
- Output variables in Layer 3 (`src/middleware/ipc.h`) omit suffix noise and are named `accel`, `gyro`, `mag`, `accel_gyro_timestamp`, `mag_timestamp`.
- Task updates wrap writes using `SYS_ENTER_CRITICAL()` / `SYS_EXIT_CRITICAL()` to ensure thread-safe atomic reader access across multi-core execution.

### Rule 5: BLE Characteristic UUID Isolation (Cross-Reference [Note 0009])
- Every distinct BLE telemetry task streams its payload notifications over its own dedicated GATT Characteristic UUID under `BLE_SERVICE_TELEM_UUID` per architectural decision [Zettel 0009](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/documents/notes/0009-distinctBleGattCharacteristicUuidIsolation.md).

---

## 3. Impact & Architectural Constraints
1. **Zero Dynamic Allocation**: All filter state objects and task stacks are allocated statically.
2. **Frequency Match**: `accel_gyro_lpf_task` runs at 500 Hz (matching `TASK_ACCEL_GYRO_DAQ_FREQ_HZ`), `mag_lpf_task` runs at 100 Hz.
3. **Traceability**: All task iterations are instrumented with `SYSVIEW_START(task_id)` and `SYSVIEW_END(task_id)` markers.
4. **BLE GATT Isolation**: Raw and filtered telemetry streams are isolated across distinct characteristic handles (`p_vec3_raw_char` and `p_vec3_filt_char`).


