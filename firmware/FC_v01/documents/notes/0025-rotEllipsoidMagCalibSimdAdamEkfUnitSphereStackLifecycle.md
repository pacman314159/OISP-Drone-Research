---
id: "0025"
title: 3D Rotated Ellipsoid Magnetometer Calibration (SIMD Adam), EKF Unit-Sphere Normalization, Task-Stack Memory Lifecycle, and FSM LED Coordination
author: Drone Research Team / Developer
date: 2026-10-14
time: 00:52:00 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#magnetometer", "#calibration", "#ellipsoid", "#simd", "#adam", "#freertos", "#fsm", "#ekf"]
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
  - "[[0026-sixPointAccelCalibSimdEllipsoidFitFsmLedArchitecture]]"
---

# Zettel 0025: 3D Rotated Ellipsoid Magnetometer Calibration (SIMD Adam), EKF Unit-Sphere Normalization, Task-Stack Memory Lifecycle, and FSM LED Coordination

## 1. Metadata & Scope
- **ID**: `0025`
- **Author**: Drone Research Team / Developer
- **Date**: 2026-10-14
- **Time**: 00:52:00 +07:00
- **Status**: Approved & Implemented
- **References**: `[Note 0000]`, `[Note 0002]`, `[Note 0003]`, `[Note 0004]`, `[Note 0008]`, `[Note 0020]`, `[Note 0023]`, `[Note 0024]`

---

## 2. Problem Statement & Engineering Constraints
Magnetometer distortion in micro-aerial vehicles consists of both hard-iron bias offsets $\mathbf{v}_0 \in \mathbb{R}^3$ (onboard permanent magnets, battery currents) and arbitrary 3D soft-iron distortion matrices $\mathbf{W} \in \mathbb{R}^{3 \times 3}$ (asymmetric ferromagnetic airframe components).

Key design tensions addressed in this specification:
1. **Full 9-DOF Rotated Ellipsoid Model**: Diagonal soft-iron assumptions (`[Note 0024]`) fail when sensor axes are not perfectly aligned with principal airframe magnetic axes. The solver must fit a fully rotated 3D ellipsoid with symmetric matrix $\mathbf{W} = \mathbf{W}^T$ (6 independent components) and hard-iron offset $\mathbf{v}_0$ (3 components).
2. **Extended Kalman Filter (EKF) Unit-Sphere Contract**: Attitude estimators (EKF/Madgwick) require pure directional unit vectors ($\|\mathbf{m}\| = 1.0$) rather than raw physical field magnitudes. Storing and projecting to an arbitrary local geomagnetic magnitude $B_0$ introduces scale biases into quaternion fusion.
3. **Zero Static Memory Overhead during Flight**: Storing 1,000 spatial 3D calibration points permanently in `.bss` consumes $12\,\text{KB}$ of internal SRAM at all times, even though calibration is a rare interactive maintenance task.
4. **Deterministic Real-Time Execution under FreeRTOS**: Large numerical optimization algorithms must not starve higher-priority real-time tasks (e.g. $500\,\text{Hz}$ flight loop) or induce FreeRTOS watchdog timeouts.
5. **Clear Pilot Interaction & Visual State Feedback**: Continuous tumble data gathering must give immediate, unambiguous optical cues for settling, active sampling, computing, completion, and abort.

---

## 3. Core Architectural Decisions

### Rule 1: Mathematical Model & Direct Unit-Sphere Mapping
The measurement model maps raw sensor readings $\mathbf{m}_{\text{raw}}$ to the unit sphere:
$$\mathbf{m}_{\text{cal}} = \mathbf{W}(\mathbf{m}_{\text{raw}} - \mathbf{v}_0), \quad \|\mathbf{m}_{\text{cal}}\|^2 = 1.0$$
- $\mathbf{v}_0$: Hard-iron bias offset vector ($\text{Gauss}$).
- $\mathbf{W}$: Symmetric soft-iron correction matrix directly mapping data to the unit sphere radius $R = 1.0$.
- In `src/core/sensor/sensor_preprocess_tasks.cpp` (`mag_preprocess_task`), filtered calibrated measurements are strictly normalized via `m_clean.normalize()`, ensuring $\|\mathbf{m}_{\text{clean}}\| = 1.0$ before publishing to the inter-task ring buffer `mag` in `middleware/ipc.h`.

### Rule 2: Time-Sliced SIMD Adam Optimizer (`ellipsoid_fit.cpp`)
The optimization problem minimizes algebraic unit residual errors across $N = 1000$ points:
$$J(\mathbf{v}_0, \mathbf{W}) = \frac{1}{N}\sum_{i=1}^N \left(\|\mathbf{W}(\mathbf{p}_i - \mathbf{v}_0)\|^2 - 1\right)^2$$
- **Hardware Acceleration**: Analytical gradients $\nabla_{\mathbf{v}_0} J$ and $\nabla_{\mathbf{W}} J$ are computed using overloaded `Matrix<float, 3, 3>` and `Vec3<float>` operators dispatching directly to Xtensa PIE SIMD instructions (`hal_simd::mat_mul_f32`, `hal_simd::dot_product`, `hal_simd::add`, `hal_simd::scale`).
- **Time-Sliced Execution**: Running inside `task_calib_mag` at $100\,\text{Hz}$ (`TASK_CALIB_MAG_FREQ_HZ`), the solver executes $2$ epochs per $10\,\text{ms}$ cycle (`CALIB_MAG_EPOCHS_PER_CYCLE = 2`). This keeps per-cycle CPU execution well below $1\,\text{ms}$, preventing FreeRTOS scheduler starvation.
- **Symmetry Enforcement**: At the conclusion of every epoch, exact matrix symmetry is enforced: $\mathbf{W} \leftarrow \frac{1}{2}(\mathbf{W} + \mathbf{W}^T)$.

### Rule 3: Zero Static Memory Overhead via Task-Stack Lifecycle
- The $12{,}000$-byte point cloud buffer `mag_cloud[CALIB_MAG_SAMPLE_COUNT]` is declared as a **local stack array** inside `task_calib_mag(void* arg)`.
- `TASK_CALIB_MAG_STACK_SIZE` is dimensioned to $20{,}480$ bytes ($20\,\text{KB}$) in `include/config.h`.
- **Lifecycle Guarantees**:
  - During normal flight, idle, preflight, or other calibration modes, `task_calib_mag` does not exist: **$0\,\text{bytes}$ consumed**.
  - Memory is allocated only when the FSM enters calibration mode and creates `task_calib_mag`.
  - When calibration finishes or the user aborts, `vTaskDelete(s_active_calib_handle)` reclaims the entire $20\,\text{KB}$ stack immediately.
  - Zero dynamic heap allocation (`malloc`/`free`) is performed.

### Rule 4: Coordinated FSM LED Gesture Sequence
Pilot interaction is orchestrated through `calib_report_queue` in `middleware/ipc.h` and the FSM state handler `handle_state_calib()`:
1. **Settling Delay ($1000\,\text{ms}$)**: `CALIB_STATUS_SETTLING` $\rightarrow$ Fast Blinking Amber (`PALETTE_AMBER`).
2. **Continuous Tumble Acquisition**: `CALIB_STATUS_SAMPLING` $\rightarrow$ Breathing Magenta (`PALETTE_MAGENTA`). Spatial gating rejects duplicate points unless $\|\Delta \mathbf{p}\|^2 \ge (0.025)^2\,\text{Gauss}^2$.
3. **User Stop Click**: Short click on USER button triggers `xTaskNotifyGive` and White Flash Burst (`PALETTE_WHITE`).
4. **Numerical Optimization Phase**: `CALIB_STATUS_COMPUTING` $\rightarrow$ Fast Blinking Magenta (`PALETTE_MAGENTA`).
5. **Success & Persistence**: `CALIB_STATUS_COMPLETED` $\rightarrow$ Solid Green (`PALETTE_GREEN`), saves 9 parameters to NVS flash (`calib_save_to_nvs`).
6. **Abort Safety**: 3-second long hold on USER button destroys task and flashes Fast Blinking Red (`PALETTE_RED`).

---

## 4. Cross-References & Traceability
- Supersedes and generalizes the diagonal-only model in `[[0024-internalFlashNvsStorageHalAndSensorCalibrationPersistence]]`.
- Implements Layer 4 math interfaces specified in `[[0008-lpfAndCalibrationTaskArchitecture]]`.
- Follows SIMD zero-fallback and operator dispatch rules established in `[[0003-simdHalArchitectureAndLayeringRule]]` and `[[0004-simdMultiTypeSupportAndBuildIntegration]]`.
- Coordinates with FSM calibration state machine in `[[0020-fsmBackbonePreFlightSensorChecksAndActiveCalibrationArchitecture]]` and `[[0023-fsmCentricCalibrationOrchestrationAndRgbLedArchitecture]]`.
- Extended to 6-point interactive accelerometer calibration in `[[0026-sixPointAccelCalibSimdEllipsoidFitFsmLedArchitecture]]`.
