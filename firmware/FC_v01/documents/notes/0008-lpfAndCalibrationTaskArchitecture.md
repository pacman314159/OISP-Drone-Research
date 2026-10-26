---
id: "0008"
title: Layer 4 Sensor Preprocessing & Calibration Pipeline Architecture Specification
author: Drone Research Team / Developer
date: 2026-09-30
time: 13:46:00 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#preprocess", "#lpf", "#simd", "#freertos", "#dsp"]
references:
  - "[[0000-firmwareStructureLayerDecision]]"
  - "[[0001-taskConfigMiddlewareIsrDecision]]"
  - "[[0002-taskEnumsAndHalAbstractionDecision]]"
  - "[[0003-simdHalArchitectureAndLayeringRule]]"
  - "[[0006-taskCreationConfigEnumsAndTaskArgsPassingDecision]]"
  - "[[0016-coreDirectoryCompactionAndDomainConsolidation]]"
  - "[[0020-fsmBackbonePreFlightSensorChecksAndActiveCalibrationArchitecture]]"
  - "[[0022-unifiedPressureRingBufferSensorVariableConsolidationAndAttitudeTaskRemoval]]"
---

# Zettel 0008: Layer 4 Sensor Preprocessing & Calibration Pipeline Architecture Specification

## 1. Metadata & Context
- **ID**: `0008`
- **Author**: Drone Research Team / Developer
- **Date**: 2026-09-30
- **Time**: 13:46:00 +07:00
- **Status**: Approved & Enforced
- **References**: `[Note 0000]`, `[Note 0003]`, `[Note 0006]`

---

## 2. Core Architectural Decisions

### Rule 1: Layer 4 Submodule Scoping (`src/core/math/` & `src/core/sensor/`)
- Digital signal processing (DSP) filters, in-flight calibration application, and offline calibration routines belong exclusively to **Layer 4 (Flight Core & Algorithms Layer)** [Note 0016].
- `src/core/math/` houses mathematical algorithms: discrete low-pass filter algorithms (`lpf.h`), matrix math (`matrix.h`), 3D ellipsoid fitting (`ellipsoid_fit.h`, `ellipsoid_fit.cpp`), and state estimators (`estimator_tasks.h`, `estimator_tasks.cpp`).
- `src/core/sensor/` houses sensor Data Acquisition (DAQ) sampling tasks (`daq_tasks.cpp`, `daq_tasks.h`), sensor preprocessing task runners (`sensor_preprocess_tasks.cpp`, `sensor_preprocess_tasks.h`), and calibration routines (`calib.cpp/.h`, `calib_tasks.cpp/.h`). Preprocessing tasks decouple raw DAQ from estimators, applying in-flight sensor bias/scale/misalignment calibration corrections (`calib_apply_*`) followed immediately by discrete low-pass filtering (`lpf_apply`) before pushing to downstream ring buffers.

### Rule 2: Tustin Bilinear Discrete Low-Pass Filters (`LowPassFilter1stOrder`, `LowPassFilter2ndOrder`, `ButterworthFilter`)
- **1st-Order Low-Pass Filter**:
  Derived from s-domain $H(s) = \frac{\omega_c}{s + \omega_c}$ via the Tustin bilinear transform ($s = \frac{2}{T} \cdot \frac{z - 1}{z + 1}$):
  $$y[i] = A \cdot (x[i] + x[i-1]) - B \cdot y[i-1]$$
  where $A = \frac{T \cdot \omega_c}{T \cdot \omega_c + 2}$, $B = \frac{T \cdot \omega_c - 2}{T \cdot \omega_c + 2}$.
- **2nd-Order Low-Pass Filter**:
  Derived from s-domain $H(s) = \frac{\omega_n^2}{s^2 + 2\zeta\omega_n s + \omega_n^2}$ with $K = T \cdot \omega_n$:
  $$y[i] = A \cdot (x[i] + 2x[i-1] + x[i-2]) - B_1 \cdot y[i-1] - B_2 \cdot y[i-2]$$
  where $D_0 = K^2 + 4\zeta K + 4$, $A = \frac{K^2}{D_0}$, $B_1 = \frac{2(K^2 - 4)}{D_0}$, $B_2 = \frac{K^2 - 4\zeta K + 4}{D_0}$.
- **2nd-Order Butterworth Filter (`ButterworthFilter`)**:
  Inherits from `LowPassFilter2ndOrder` with maximally flat damping ratio $\zeta = \frac{1}{\sqrt{2}} \approx 0.70710678\text{f}$.
- State classes and structures are named using `PascalCase`, containing history vectors and gain coefficients.
- State instances are declared task-locally to prevent state cross-contamination across sensor channels.
- **Flight IMU Filter Allocations (`sensor_preprocess_tasks.cpp`)**:
  - **Gyroscope**: 2nd-order LPF (`LowPassFilter2ndOrder`) with $\omega_n = 50\,\text{rad/s}$ (`GYRO_LPF_CUTOFF_WN_RAD_S`) and damping ratio $\zeta = 0.55$ (`GYRO_LPF_ZETA`), sampled at $500\,\text{Hz}$ ($f_c \approx 7.958\,\text{Hz}$).
  - **Accelerometer**: 1st-order LPF (`LowPassFilter1stOrder`) with cutoff $f_c = 30\,\text{Hz}$ (`ACCEL_LPF_CUTOFF_FREQ_HZ`), sampled at $500\,\text{Hz}$.
  - **Magnetometer**: 1st-order LPF (`LowPassFilter1stOrder`) with cutoff $f_c = 10\,\text{Hz}$ (`MAG_LPF_CUTOFF_FREQ_HZ`), sampled at $100\,\text{Hz}$.

### Rule 3: Native Vector Math Acceleration via `Vec3<float>`
- Filter difference equations are formulated cleanly using `Vec3<float>` operator overloads (`+`, `-`, `*`).
- Because `Vec3<float>` inherits `Matrix<float, 3, 1>`, operations automatically dispatch to target SIMD primitives (`hal_simd::add`, `hal_simd::sub`, `hal_simd::scale`) under `ENABLE_SIMD_ACCELERATION`, eliminating manual `#if (!ENABLE_SIMD_ACCELERATION)` wrappers in `lpf.h`.

### Rule 4: IPC Clean Variable Naming & Ring Buffer Consolidation
- Filtered sensor ring buffers in Layer 3 (`src/middleware/ipc.h`) omit suffix noise and are named `accel`, `gyro`, `mag`, along with timestamps `filtered_accel_gyro_timestamp`, `filtered_mag_timestamp`.
- Downstream tasks read thread-safely directly from ring buffers (`accel.front()` or `accel.get_snapshot()`), eliminating redundant orphan global variables.

### Rule 5: BLE Characteristic UUID Isolation (Cross-Reference [Note 0009])
- Every distinct BLE telemetry task streams its payload notifications over its own dedicated GATT Characteristic UUID under `BLE_SERVICE_TELEM_UUID` per architectural decision [Zettel 0009](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/documents/notes/0009-distinctBleGattCharacteristicUuidIsolation.md).

---

## 3. Impact & Architectural Constraints
1. **Zero Dynamic Allocation**: All filter state objects and task stacks are allocated statically.
2. **Frequency Match**: `accel_gyro_preprocess_task` runs at 500 Hz (matching `TASK_ACCEL_GYRO_DAQ_FREQ_HZ`), `mag_preprocess_task` runs at 100 Hz.
3. **Traceability**: All task iterations are instrumented with `SYSVIEW_START(task_id)` and `SYSVIEW_END(task_id)` markers.
4. **BLE GATT Isolation**: Raw and filtered telemetry streams are isolated across distinct characteristic handles (`p_vec3_raw_char` and `p_vec3_filt_char`).
5. **FSM & Calibration Integration**: Cross-reference [[0020-fsmBackbonePreFlightSensorChecksAndActiveCalibrationArchitecture]] for the 100 Hz FSM coordinator, pre-flight stationary detection, and active/passive calibration lifecycle.


