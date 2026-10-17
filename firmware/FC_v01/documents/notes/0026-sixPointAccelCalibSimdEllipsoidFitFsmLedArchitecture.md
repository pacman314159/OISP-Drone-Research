---
id: "0026"
title: 6-Point Accelerometer Calibration via SIMD-Accelerated 6-Parameter Axis-Aligned Ellipsoid Fit, Autonomous Stationary Gating, and FSM LED Coordination
author: Drone Research Team / Developer
date: 2026-10-14
time: 18:05:00 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#accelerometer", "#calibration", "#ellipsoid", "#simd", "#adam", "#freertos", "#fsm"]
references:
  - "[[0000-firmwareStructureLayerDecision]]"
  - "[[0001-taskConfigMiddlewareIsrDecision]]"
  - "[[0002-taskEnumsAndHalAbstractionDecision]]"
  - "[[0003-simdHalArchitectureAndLayeringRule]]"
  - "[[0004-simdMultiTypeSupportAndBuildIntegration]]"
  - "[[0008-lpfAndCalibrationTaskArchitecture]]"
  - "[[0020-fsmBackbonePreFlightSensorChecksAndActiveCalibrationArchitecture]]"
  - "[[0023-fsmCentricCalibrationOrchestrationAndRgbLedArchitecture]]"
  - "[[0024-internalFlashNvsStorageHalAndSensorCalibrationPersistence]]"
  - "[[0025-rotEllipsoidMagCalibSimdAdamEkfUnitSphereStackLifecycle]]"
---

# Zettel 0026: 6-Point Accelerometer Calibration via SIMD-Accelerated 6-Parameter Axis-Aligned Ellipsoid Fit, Autonomous Stationary Gating, and FSM LED Coordination

## 1. Metadata & Scope
- **ID**: `0026`
- **Author**: Drone Research Team / Developer
- **Date**: 2026-10-14
- **Time**: 18:05:00 +07:00
- **Status**: Approved & Implemented
- **References**: `[Note 0000]`, `[Note 0002]`, `[Note 0003]`, `[Note 0008]`, `[Note 0020]`, `[Note 0023]`, `[Note 0024]`, `[Note 0025]`

---

## 2. Problem Statement & Engineering Constraints
Triaxial MEMS accelerometers in flight control hardware suffer from non-zero zero-g bias offsets $\mathbf{v}_0 \in \mathbb{R}^3$ and per-axis sensitivity scaling errors $\mathbf{s} \in \mathbb{R}^3$.

Key design considerations and mathematical constraints:
1. **The 9-DOF Rotated Ellipsoid Degeneracy on 6 Points**: A full 3D rotated ellipsoid has 9 degrees of freedom ($3$ center offsets $+ 3$ semi-axis radii $+ 3$ 3D spatial rotation angles). Mathematically, fitting a 9-DOF quadric requires at least 9 independent spatial orientations. When presented with only 6 discrete orthogonal clusters (the 6 faces of an IMU tumble), the off-diagonal cross-axis terms are underdetermined, permitting an infinite number of rotated ellipsoids to fit the dataset and causing the optimization to deform or diverge unpredictably.
2. **Physical Independence of Accelerometer Axes**: Unlike magnetometers—which experience substantial soft-iron distortion and cross-axis rotation from asymmetric ferromagnetic airframe components—MEMS accelerometers have negligible cross-axis rotation. The dominant deterministic errors are strictly zero-g offset ($\mathbf{v}_0$) and main-axis sensitivity scaling ($\mathbf{s}$).
3. **Strictly Determined 6-Parameter Model**: Fitting exactly 6 parameters ($\mathbf{v}_0 \in \mathbb{R}^3$ and $\mathbf{s} \in \mathbb{R}^3$) to a 6-face dataset ($300$ samples across 6 orthogonal poses) is strictly determined, robustly convex, and mathematically unique.
4. **Attitude-Independent Pose Divergence**: In pre-flight calibration mode, the attitude estimation filter (EKF/Madgwick) is uninitialized. The calibration state machine must detect pose distinctness ($\ge 45^\circ$ angular separation) directly from raw normalized acceleration vectors without relying on attitude quaternions.
5. **Task-Stack Memory Lifecycle**: Buffering 300 samples across 6 poses permanently in `.bss` wastes valuable internal SRAM during normal flight. All sample buffers must exist strictly on the worker task stack and vanish upon task destruction.

---

## 3. Core Architectural Decisions

### Rule 1: Mathematical Model & 6-Parameter Axis-Aligned Solver
The measurement model maps raw acceleration readings $\mathbf{a}_{\text{raw}}$ (in $g$) to calibrated values $\mathbf{a}_{\text{cal}}$:
$$\mathbf{a}_{\text{cal}} = (\mathbf{a}_{\text{raw}} - \mathbf{v}_0) \odot \mathbf{s}, \quad \|\mathbf{a}_{\text{cal}}\|^2 = 1.0\,g^2$$
- $\mathbf{v}_0 \in \mathbb{R}^3$: Zero-g bias offset vector ($g$).
- $\mathbf{s} \in \mathbb{R}^3$: Principal-axis sensitivity scaling vector ($g^{-1}$), mapping 1g resting measurements directly to the unit sphere radius $R = 1.0$.
- In `src/middleware/ipc.h`, `SensorCalibrationData` stores `accel_bias` and `accel_scale` as `Vec3<float>`.
- In `src/core/sensor/calib.cpp`, `calib_apply_accel()` executes the vector Hadamard product via hardware SIMD vector instructions.

### Rule 2: Analytical Gradients for 6-Parameter Adam Optimizer
The cost function minimizes algebraic unit-sphere residual errors across $n = 300$ samples:
$$e(\mathbf{v}_0, \mathbf{s}) = \frac{1}{n} \sum_{i=1}^n r_i^2 = \frac{1}{n} \sum_{i=1}^n \left(\|\mathbf{s} \odot (\mathbf{p}_i - \mathbf{v}_0)\|^2 - 1.0\right)^2$$
For each sample $i$:
$$\mathbf{d}_i = \mathbf{p}_i - \mathbf{v}_0, \quad \mathbf{u}_i = \mathbf{s} \odot \mathbf{d}_i, \quad r_i = \|\mathbf{u}_i\|^2 - 1.0$$
The analytical gradients are derived via multivariate chain rule:
$$\nabla_{\mathbf{v}_0} e = -\frac{4}{n} \mathbf{s} \odot \left( \sum_{i=1}^n r_i \mathbf{u}_i \right) = -\frac{4}{n} \mathbf{s} \odot \mathbf{s}_{\mathbf{u}}$$
$$\nabla_{\mathbf{s}} e = \frac{4}{n} \sum_{i=1}^n r_i (\mathbf{u}_i \odot \mathbf{d}_i) = \frac{4}{n} \mathbf{g}_{\mathbf{s}}$$
where $\mathbf{s}_{\mathbf{u}} \in \mathbb{R}^3$ and $\mathbf{g}_{\mathbf{s}} \in \mathbb{R}^3$ are SIMD-accelerated vector accumulators.

### Rule 3: Architectural Separation of 6-Param and 9-Param Solvers
To maintain symmetry and clear architectural domain separation in `src/core/math/ellipsoid_fit.h` and `ellipsoid_fit.cpp`:
- **9-Parameter Rotated Solver (Magnetometer)**:
  - `Ellipsoid9ParamState`, `Ellipsoid9ParamResult`
  - `ellipsoid_fit_9param_init()`, `ellipsoid_fit_9param_step()`, `ellipsoid_fit_9param_get_params()`
- **6-Parameter Axis-Aligned Solver (Accelerometer)**:
  - `Ellipsoid6ParamState`, `Ellipsoid6ParamResult`
  - `ellipsoid_fit_6param_init()`, `ellipsoid_fit_6param_step()`, `ellipsoid_fit_6param_get_params()`

### Rule 4: Attitude-Independent Gravity Vector Divergence & Stationary Gating
To verify orientation distinctness without an active attitude estimator:
- **Resting Stability Hold**: The aircraft is considered stationary in an orientation when consecutive raw accelerometer readings stay within a tolerance bound of $\pm 0.03\,g$ for $2.0\,\text{s}$ ($400$ samples at $200\,\text{Hz}$).
- **Angular Divergence Verification**: For each candidate stationary pose $\mathbf{p}_{\text{cand}}$, its normalized dot product against all previously locked poses $\mathbf{p}_{\text{locked}, j}$ ($j = 0 \dots k-1$) must satisfy:
  $$\cos\theta = \frac{\mathbf{p}_{\text{cand}} \cdot \mathbf{p}_{\text{locked}, j}}{\|\mathbf{p}_{\text{cand}}\| \|\mathbf{p}_{\text{locked}, j}\|} \le \cos(45^\circ) \approx 0.7071$$
- If $\cos\theta > 0.7071$, the pose is rejected as redundant, and the system waits until the pilot rotates to a distinct face.

### Rule 5: Motion Unlatching & Inter-Orientation State Machine
After an orientation is locked:
1. `task_calib_accel` records $50$ samples into the point-cloud buffer and computes the mean centroid $\mathbf{p}_{\text{locked}, k}$.
2. The task transmits `CALIB_STATUS_STEP_LOCKED` to `calib_report_queue`, triggering a White flash burst on the status LED (`LED_PATTERN_FLASH_BURST`, `PALETTE_WHITE`).
3. To prevent immediately re-recording the same stationary position, the task unlatches only after physical motion is detected:
   $$\|\mathbf{a}_{\text{raw}} - \mathbf{p}_{\text{locked}, k}\| > 0.10\,g$$
4. Once motion is detected, the task sends `CALIB_STATUS_SAMPLING`, restoring breathing Orange LED (`LED_PATTERN_BREATHE`, `PALETTE_ORANGE`), and re-arms the stationary hold detector for the next pose.

### Rule 6: Task-Stack Lifecycle & Zero Static Overhead
- The point cloud buffer `accel_cloud[CALIB_ACCEL_SAMPLE_COUNT]` ($300 \times 12 = 3600\,\text{bytes}$) is allocated as a local stack variable inside `task_calib_accel`.
- `TASK_CALIB_ACCEL_STACK_SIZE` is dimensioned to $12{,}288\,\text{bytes}$ ($12\,\text{KB}$) in `include/config.h`.
- The task is spawned dynamically by `TASK_FSM` upon menu selection and passively halts via `ulTaskNotifyTake(pdTRUE, portMAX_DELAY)` upon completion.
- `TASK_FSM` deletes the task handle via `vTaskDelete(s_active_calib_handle)`, reclaiming the entire $12\,\text{KB}$ stack immediately. Static `.bss` overhead during normal flight is $0\,\text{bytes}$.

### Rule 7: FSM Visual LED Gesture Coordination
- **Entry Settling ($2000\,\text{ms}$)**: `CALIB_STATUS_SETTLING` $\rightarrow$ Fast Blinking Amber (`PALETTE_AMBER`).
- **Interactive Tumble Pacing**: `CALIB_STATUS_SAMPLING` $\rightarrow$ Breathing Orange (`PALETTE_ORANGE`).
- **Pose Captured ACK**: `CALIB_STATUS_STEP_LOCKED` $\rightarrow$ White Flash Burst (`PALETTE_WHITE`).
- **Numerical Optimization**: `CALIB_STATUS_COMPUTING` $\rightarrow$ Fast Blinking Orange (`PALETTE_ORANGE`).
- **Completion & Persistence**: `CALIB_STATUS_COMPLETED` $\rightarrow$ Solid Green (`PALETTE_GREEN`).

---

## 4. Cross-References & Traceability
- Mathematical formulation derived from [`documents/Magnetometer Ellipsoid Calibration SIMD.md`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/documents/Magnetometer%20Ellipsoid%20Calibration%20SIMD.md) under diagonal soft-iron constraints.
- Complements the 9-parameter rotated magnetometer calibration model in `[[0025-rotEllipsoidMagCalibSimdAdamEkfUnitSphereStackLifecycle]]`.
- Persists to Flash NVS via `FlashStorageDriver` per `[[0024-internalFlashNvsStorageHalAndSensorCalibrationPersistence]]`.
- Coordinates with FSM calibration state machine in `[[0020-fsmBackbonePreFlightSensorChecksAndActiveCalibrationArchitecture]]` and `[[0023-fsmCentricCalibrationOrchestrationAndRgbLedArchitecture]]`.
