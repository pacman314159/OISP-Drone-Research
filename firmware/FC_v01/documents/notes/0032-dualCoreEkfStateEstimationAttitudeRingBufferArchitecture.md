---
id: "0032"
title: Dual-Core 7-State Quaternion Extended Kalman Filter (EKF), SIMD Hardware Acceleration, Unit-Sphere Geomagnetic Fusion, and Inter-Task Attitude Ring Buffers
author: Drone Research Team / Developer
date: 2026-10-17
time: 03:15:00 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#ekf", "#kalman", "#quaternion", "#attitude", "#freertos", "#simd", "#ipc"]
references:
  - "[[0000-firmwareStructureLayerDecision]]"
  - "[[0001-taskConfigMiddlewareIsrDecision]]"
  - "[[0002-taskEnumsAndHalAbstractionDecision]]"
  - "[[0003-simdHalArchitectureAndLayeringRule]]"
  - "[[0004-simdMultiTypeSupportAndBuildIntegration]]"
  - "[[0007-systemViewTracingAndRTOSTimingAnalysisConventions]]"
  - "[[0008-lpfAndCalibrationTaskArchitecture]]"
  - "[[0010-bleMinimumPacketFrequencyAndDualTriggerFlush]]"
  - "[[0016-coreDirectoryCompactionAndDomainConsolidation]]"
  - "[[0019-universalFlightDataAnalyzerAndBinaryTelemetryAlignmentSpecification]]"
  - "[[0021-bleTelemetryDispatcherConsolidationAndSubscriptionGating]]"
  - "[[0022-unifiedPressureRingBufferSensorVariableConsolidationAndAttitudeTaskRemoval]]"
  - "[[0025-rotEllipsoidMagCalibSimdAdamEkfUnitSphereStackLifecycle]]"
  - "[[0026-sixPointAccelCalibSimdEllipsoidFitFsmLedArchitecture]]"
  - "[[0029-unificationOfInterTaskRingBufferSizeConstant]]"
  - "[[0031-sixGatePreflightTerminalDevelopmentAndTaskEncapsulationArchitecture]]"
---

# Zettel 0032: Dual-Core 7-State Quaternion Extended Kalman Filter (EKF), SIMD Hardware Acceleration, Unit-Sphere Geomagnetic Fusion, and Inter-Task Attitude Ring Buffers

## 1. Metadata & Scope
- **ID**: `0032`
- **Author**: Drone Research Team / Developer
- **Date**: 2026-10-17
- **Time**: 03:15:00 +07:00
- **Status**: Approved & Implemented
- **References**: `[Note 0000]`, `[Note 0001]`, `[Note 0002]`, `[Note 0003]`, `[Note 0007]`, `[Note 0008]`, `[Note 0019]`, `[Note 0022]`, `[Note 0025]`, `[Note 0026]`

---

## 2. Problem Statement & Architecture Tensions

1. **State Estimation Deficit Following Ghost Task Removal ([Note 0022])**:
   Zettel 0022 deprecated a legacy placeholder `TASK_ATTITUDE` because no concrete mathematical estimator existed. Flight control and downstream guidance require real-time 3D attitude estimation ($\text{roll } \phi, \text{pitch } \theta, \text{yaw } \psi$ and quaternion $\mathbf{q}$) along with dynamic gyroscope bias compensation.
2. **Multi-Rate Asynchronous Sensor Dynamics**:
   The primary flight IMU (MPU6050) samples at $500\,\text{Hz}$ on I2C0, whereas the magnetometer (HMC5883L) samples at $100\,\text{Hz}$. Running magnetic matrix operations ($3\times 7$ Jacobian, $3\times 3$ inversion) synchronously within the $500\,\text{Hz}$ flight loop wastes precious execution budget ($2\,\text{ms}$ limit).
3. **Local Geomagnetic Dip Distortion**:
   Assuming a purely horizontal geomagnetic field ($\mathbf{b}_{\text{earth}} = [1, 0, 0]^T$) causes heading pitch/roll coupling errors. In Ho Chi Minh City ($10.75^\circ\,\text{N}, 106.67^\circ\,\text{E}$), the magnetic inclination is $+9.78^\circ$, yielding a non-zero vertical component $b_z \approx 0.16985$.
4. **Empirical Noise Grounding**:
   Speculative process and measurement noise covariance parameters ($Q, R_a, R_m$) risk filter sluggishness or divergence. Covariances must be anchored directly in empirical flight data distributions ([`analyzers/log/0001-LOG.txt`](file:///d:/OneDrive_MSFT/Work/Drone_Research/analyzers/log/0001-LOG.txt)).

---

## 3. Core Architectural Decisions

### Rule 1: Mathematical Model (7-State Quaternion EKF)
The state vector $\mathbf{x} \in \mathbb{R}^7$ comprises the orientation quaternion $\mathbf{q} = [q_0, q_1, q_2, q_3]^T$ (Earth/NED to Body) and gyroscope zero-rate biases $\mathbf{b} = [b_x, b_y, b_z]^T$ in $\text{rad/s}$:
$$\mathbf{x} = \begin{bmatrix} q_0 & q_1 & q_2 & q_3 & b_x & b_y & b_z \end{bmatrix}^T$$

1. **State Prediction (`predict(dt, gyro)`)**:
   - Angular rates: $\boldsymbol{\omega} = \boldsymbol{\omega}_{\text{raw}} \cdot \frac{\pi}{180} - \mathbf{b}$ (using `DEG_TO_RAD`).
   - Kinematic transition matrix $\mathbf{A} = \mathbf{I}_{4\times 4} + \frac{\Delta t}{2}\boldsymbol{\Omega}(\boldsymbol{\omega})$.
   - Quaternion state propagation: $\mathbf{q}_{k+1} = \mathbf{A}\mathbf{q}_k$ (accelerated via hardware SIMD `hal_simd::mat_mul_f32`).
   - $7\times 7$ Jacobian $\mathbf{F}$ is computed analytically.
   - Covariance propagation via SIMD: $\mathbf{P} \leftarrow \mathbf{F}\mathbf{P}\mathbf{F}^T + \mathbf{Q}$.
2. **Accelerometer Gravity Measurement Update (`update_accel(accel_g)`)**:
   - Accelerometer reading is normalized via SIMD dot product and scaling: $\mathbf{z}_a = \frac{\mathbf{a}}{\|\mathbf{a}\|}$.
   - Gravity observation model: $\mathbf{h}_a = [2(q_1 q_3 - q_0 q_2), 2(q_0 q_1 + q_2 q_3), q_0^2 - q_1^2 - q_2^2 + q_3^2]^T$.
   - Innovation $\mathbf{y}_a = \mathbf{z}_a - \mathbf{h}_a$, $\mathbf{S}_a = \mathbf{H}_a \mathbf{P} \mathbf{H}_a^T + \mathbf{R}_a$, $\mathbf{K}_a = \mathbf{P}\mathbf{H}_a^T \mathbf{S}_a^{-1}$.
   - Closed-form $3\times 3$ algebraic inversion (`hal_simd::inv3x3_f32`) ensures sub-microsecond determinism.
   - Post-update quaternion normalization via `Vec4::normalize()` (`hal_simd::dot_product` and `hal_simd::scale`).
3. **Magnetometer Measurement Update (`update_mag(mag_unit)`)**:
   - Pre-calibrated unit-sphere vector $\mathbf{z}_m$ satisfies $\|\mathbf{z}_m\| = 1.0$ per Note 0025.
   - Destination: Ho Chi Minh City, Vietnam ($10.75^\circ\,\text{N}, 106.67^\circ\,\text{E}$).
   - Magnetic dip angle: $\text{MAGNETIC\_DIP} = 9.78^\circ \times \frac{\pi}{180} = 0.17069\,\text{rad}$.
   - Reference field: $\mathbf{b}_{\text{earth}} = [\cos(\text{MAGNETIC\_DIP}), 0, \sin(\text{MAGNETIC\_DIP})]^T = [0.98546, 0, 0.16985]^T$ with $\|\mathbf{b}_{\text{earth}}\|^2 = 1.0$.
   - Magnetic observation model: $\mathbf{h}_m = \mathbf{R}(\mathbf{q})^T \mathbf{b}_{\text{earth}}$.
   - Innovation $\mathbf{y}_m = \mathbf{z}_m - \mathbf{h}_m$, $\mathbf{S}_m = \mathbf{H}_m \mathbf{P} \mathbf{H}_m^T + \mathbf{R}_m$, $\mathbf{K}_m = \mathbf{P}\mathbf{H}_m^T \mathbf{S}_m^{-1}$.

### Rule 2: Dual-Core Asynchronous RTOS Partitioning (Option B)
To align with the system RTOS architecture specification ([`RTOS Architecture.pdf`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/documents/RTOS Architecture.pdf)):
- **Pilot Task (`ekf_roll_pitch_task`)**: Executes at $500\,\text{Hz}$ (Priority 3). Consumes `accel` and `gyro` ring buffers, performs state prediction and accelerometer gravity correction, extracts attitude quaternions and Euler angles, and pushes to IPC ring buffers.
- **Navigator Task (`ekf_yaw_task`)**: Executes at $100\,\text{Hz}$ (Priority 2). Checks `filtered_mag_timestamp` and performs magnetometer heading correction.
- **Inter-Task Synchronization**: Both tasks synchronize state access using `ekf_state_mutex` with a $5\,\text{ms}$ timeout guard (`pdMS_TO_TICKS(5)`).
- **Core Affinity**: Declared with `tskNO_AFFINITY` in `include/config.h`, allowing the FreeRTOS SMP scheduler to distribute work dynamically.

### Rule 3: Inter-Task IPC Ring Buffers (Layer 3)
Layer 3 (`src/middleware/ipc.h` and `ipc.cpp`) exposes thread-safe ring buffers:
```cpp
extern RingBuffer<Vec4, RING_BUF_SIZE>        att_quat;      // Unit quaternion [q0, q1, q2, q3]
extern RingBuffer<Vec3<float>, RING_BUF_SIZE> att_euler;     // Euler angles [roll, pitch, yaw] (deg)
extern RingBuffer<uint32_t, RING_BUF_SIZE>    att_timestamp; // Microseconds (us)
extern SemaphoreHandle_t                      ekf_state_mutex;
```

### Rule 4: Empirical Sensor Noise Calibration
Sensor noise parameters in `include/config.h` are grounded in flight log session `LOG_0032` ([`analyzers/log/0001-LOG.txt`](file:///d:/OneDrive_MSFT/Work/Drone_Research/analyzers/log/0001-LOG.txt)):
- Accel noise variances: $R_{ax} = 2.25 \times 10^{-6}$, $R_{ay} = 1.69 \times 10^{-6}$, $R_{az} = 4.84 \times 10^{-6}$.
- Gyro rate variances: $\sigma_{gx}^2 = 1.641 \times 10^{-6}$, $\sigma_{gy}^2 = 2.921 \times 10^{-6}$, $\sigma_{gz}^2 = 0.563 \times 10^{-6}\text{ (rad/s)}^2$.
- Mag noise variances: $R_{mx} = 6.76 \times 10^{-6}$, $R_{my} = 4.84 \times 10^{-6}$, $R_{mz} = 4.49 \times 10^{-5}$.

### Rule 5: Memory Placement & RAM Access Latency (Internal SRAM vs External PSRAM)
To guarantee the strict $500\,\text{Hz}$ ($2000\,\mu\text{s}$) flight loop WCET without jitter or bus contention:
1. **Persistent State & Covariance Matrices (`_x`, `_P`, `_Q`, `_Ra`, `_Rm`)**:
   - Reside within static global instance `AttitudeEKF g_ekf;` in `src/core/math/estimator_tasks.cpp` (~$500\,\text{bytes}$).
   - Placed in `.bss` / `.data` which maps directly to **Internal Data RAM (DRAM, SRAM1/SRAM2)** via the direct internal D-Bus at $240\,\text{MHz}$ ($0\text{ wait-states}$, $\sim 4.16\,\text{ns}$ single-cycle access).
2. **Intermediate Calculation Matrices (`F`, `H_a`, `H_m`, `S`, `K`, `A`)**:
   - Allocated on the FreeRTOS task stacks in `predict()`, `update_accel()`, and `update_mag()`.
   - Task stacks (`TASK_EKF_ROLL_PITCH`: $8\,\text{KB}$, `TASK_EKF_YAW`: $4\,\text{KB}$) are allocated via `xTaskCreatePinnedToCore`, which FreeRTOS sources exclusively from internal SRAM (`MALLOC_CAP_INTERNAL`).
3. **PSRAM Exclusion**:
   - External Octal PSRAM ($8\,\text{MB}$ OPI at $80\,\text{MHz}$ DDR) is accessed via the CPU's $32\,\text{KB}$ L1 cache, where cache misses cost $20\text{ to }50+\text{ CPU cycles}$.
   - Because `CONFIG_SPIRAM_ALLOW_BSS_SEG_EXTERNAL_MEMORY=n` and `CONFIG_SPIRAM_ALLOW_STACK_EXTERNAL_MEMORY=n` in `sdkconfig.defaults`, the linker strictly isolates all EKF state and stack execution inside zero-wait-state internal SRAM.

---

## 4. Cross-References & Traceability
- [[0000-firmwareStructureLayerDecision]]: Layer 4 algorithms encapsulation in `src/core/math/estimator_tasks.*`.
- [[0001-taskConfigMiddlewareIsrDecision]]: Centralized `ekf_state_mutex` in `init_ipc()`.
- [[0002-taskEnumsAndHalAbstractionDecision]]: Centralized task parameters in `include/config.h`.
- [[0003-simdHalArchitectureAndLayeringRule]]: SIMD matrix operations (`hal_simd::inv3x3_f32`, `mat_mul_f32`).
- [[0007-systemViewTracingAndRTOSTimingAnalysisConventions]]: `SYSVIEW_START` / `SYSVIEW_END` markers and overrun guards.
- [[0008-lpfAndCalibrationTaskArchitecture]]: Upstream consumption of pre-filtered, calibrated sensor streams.
- [[0019-universalFlightDataAnalyzerAndBinaryTelemetryAlignmentSpecification]]: Binary blackbox logging structs `PktAttQuat` and `PktAttEuler`.
- [[0022-unifiedPressureRingBufferSensorVariableConsolidationAndAttitudeTaskRemoval]]: Formal replacement of ghost attitude task with dual-core EKF engine.
- [[0025-rotEllipsoidMagCalibSimdAdamEkfUnitSphereStackLifecycle]]: Unit-sphere magnetometer contract ($\|\mathbf{m}\| = 1.0$).
- [[0029-unificationOfInterTaskRingBufferSizeConstant]]: Unified `RING_BUF_SIZE = 10` for attitude buffers.
- [[0031-sixGatePreflightTerminalDevelopmentAndTaskEncapsulationArchitecture]]: Downstream attitude consumption for preflight safety checks and control loops.
