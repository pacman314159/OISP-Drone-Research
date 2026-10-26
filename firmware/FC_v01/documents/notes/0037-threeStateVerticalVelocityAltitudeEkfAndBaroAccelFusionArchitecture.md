---
id: "0037"
title: 3-State Vertical Velocity & Altitude Extended Kalman Filter (AltitudeEKF), Nonlinear Baro Observation & Layer 4 RTOS Task Architecture Specification
author: Drone Research Team / Developer
date: 2026-10-26
time: 08:00:00 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#ekf", "#math", "#barometer", "#altitude", "#indi", "#freertos"]
references:
  - "[[0000-firmwareStructureLayerDecision]]"
  - "[[0001-taskConfigMiddlewareIsrDecision]]"
  - "[[0002-taskEnumsAndHalAbstractionDecision]]"
  - "[[0017-bmp180TwoStepI2cTransactionAndBaroDaqArchitecture]]"
  - "[[0020-fsmBackbonePreFlightSensorChecksAndActiveCalibrationArchitecture]]"
  - "[[0022-unifiedPressureRingBufferSensorVariableConsolidationAndAttitudeTaskRemoval]]"
  - "[[0023-fsmCentricCalibrationOrchestrationAndRgbLedArchitecture]]"
  - "[[0026-sixPointAccelCalibSimdEllipsoidFitFsmLedArchitecture]]"
  - "[[0031-sixGatePreflightTerminalDevelopmentAndTaskEncapsulationArchitecture]]"
  - "[[0032-dualCoreEkfStateEstimationAttitudeRingBufferArchitecture]]"
  - "[[0036-layer4SafetyDomainAndPreflightGatesDecoupling]]"
---

# Zettel 0037: 3-State Vertical Velocity & Altitude Extended Kalman Filter (AltitudeEKF), Nonlinear Baro Observation & Layer 4 RTOS Task Architecture Specification

## 1. Metadata & Scope
- **ID**: `0037`
- **Author**: Drone Research Team / Developer
- **Date**: 2026-10-26
- **Time**: 08:00:00 +07:00
- **Status**: Approved & Implemented
- **References**: `[Note 0000]`, `[Note 0001]`, `[Note 0002]`, `[Note 0017]`, `[Note 0022]`, `[Note 0026]`, `[Note 0032]`, `[Note 0036]`

---

## 2. Problem Statement & Motivation
Prior to this implementation, the `FC_v01` firmware estimated full 3-axis attitude (quaternions and Euler angles via `AttitudeEKF` in `[Note 0032]`), but lacked vertical state estimation:
1. **Vertical Flight Control Prerequisite**: The upcoming 4-DOF Adaptive Incremental Nonlinear Dynamic Inversion (INDI) controller requires high-bandwidth, drift-free vertical velocity ($\hat{v}_z$) and bias-compensated inertial vertical acceleration ($\ddot{z}_I$) to stabilize climb and descent rates.
2. **Sensor Complementarity**: The BMP180 barometric pressure sensor provides absolute altitude reference without long-term drift, but suffers from low sampling cadence ($50\,\text{Hz}$), acoustic/propwash noise ($\sigma_P \approx 4.22\,\text{Pa} \implies \sigma_z \approx 0.35\,\text{m}$), and latency ($13.5\,\text{ms}$ conversion time). Conversely, the body-mounted accelerometer provides instantaneous acceleration at $500\,\text{Hz}$, but double-integrating raw vertical acceleration diverges rapidly due to sensor bias, tilt errors, and gravity leakage.
3. **Local Atmospheric Reference Drift**: Day-to-day weather shifts cause ambient sea-level pressure to fluctuate by hundreds of Pascals ($> 10\,\text{m}$ apparent altitude drift). The system requires deterministic local ground pressure calibration ($P_0$) during both active pre-flight sensor calibration (`CALIB` state) and launch-pad readiness (`READY` state).

---

## 3. Mathematical & Algorithmic Formulation

```
[Layer 4: 500 Hz IMU Preprocess] ──> [accel (g)] ──────────┐
                                                           │
[Layer 4: 500 Hz AttitudeEKF] ───> [att_quat (q)] ────────┼──> 100 Hz Predict Step:
                                                           │     a_z,raw = (r3(q) . a_B - 1) * 9.80665
                                                           │     x_pred = F * x + B * a_z,inertial
                                                           │     P_pred = F * P * F^T + Q
                                                           ▼
[Layer 4: 50 Hz Baro DAQ] ───────> [pressure (Pa)] ───────┼──> 50 Hz Update Step:
                                                           │     h(x) = P0 * (1 - z / 44330)^5.255
                                                           │     H = [dH/dz, 0, 0]
                                                           │     K = P * H^T * (H * P * H^T + R)^-1
                                                           ▼
[Layer 3 IPC: alt_state & alt_timestamp] <─────────────────┘     Outputs: [z, v_z, a_z,inertial]
```

### A. Coordinate Frame & Kinematic Acceleration (Z-Up)
Per `[Note 0026]` and `[Note 0032]`, the Level Coordinate frame follows the Z-Up convention ($z > 0$ upward).
When stationary and level ($\mathbf{q} = [1, 0, 0, 0]^T$), the calibrated accelerometer measures the upward normal reaction force $\mathbf{a}_B \approx [0, 0, +1.0]^T\,g$.
The direction of the inertial vertical axis in the body frame is the 3rd row of the rotation matrix $\mathbf{R}_B^I(\mathbf{q})$:
$$\mathbf{r}_3(\mathbf{q}) = \begin{bmatrix} 2(q_1 q_3 - q_0 q_2) \\ 2(q_0 q_1 + q_2 q_3) \\ q_0^2 - q_1^2 - q_2^2 + q_3^2 \end{bmatrix}$$

Projecting $\mathbf{a}_B$ onto $\mathbf{r}_3(\mathbf{q})$ and subtracting static $1.0\,g$ reaction:
$$a_{z,\text{raw}} = \left( \mathbf{r}_3(\mathbf{q}) \cdot \mathbf{a}_B - 1.0\text{f} \right) \cdot G_{\text{std}}, \quad G_{\text{std}} = 9.80665\,\text{m/s}^2$$

Subtracting the estimated vertical accelerometer bias $b_{a,z}$:
$$\ddot{z}_I = a_{z,\text{inertial}} = a_{z,\text{raw}} - b_{a,z} \quad [\text{m/s}^2]$$

### B. 3-State Extended Kalman Filter Equations
- **State Vector**:
  $$\mathbf{x} = \begin{bmatrix} z \\ v_z \\ b_{a,z} \end{bmatrix} \in \mathbb{R}^3$$
  where $z$ is altitude Above Ground Level (AGL, $\text{m}$), $v_z$ is vertical climb rate ($\text{m/s}$), and $b_{a,z}$ is vertical accelerometer bias ($\text{m/s}^2$).

- **Predict Step**:
  $$\mathbf{x}_{k|k-1} = \begin{bmatrix} z + v_z \Delta t + \frac{1}{2}(a_{z,\text{raw}} - b_{a,z})\Delta t^2 \\ v_z + (a_{z,\text{raw}} - b_{a,z})\Delta t \\ b_{a,z} \end{bmatrix}$$
  Jacobian:
  $$\mathbf{F} = \begin{bmatrix} 1 & \Delta t & -\frac{1}{2}\Delta t^2 \\ 0 & 1 & -\Delta t \\ 0 & 0 & 1 \end{bmatrix}$$
  Covariance propagation (accelerated via `Matrix<float, 3, 3>` in `matrix.h`):
  $$\mathbf{P}_{k|k-1} = \mathbf{F}\mathbf{P}_{k-1}\mathbf{F}^T + \mathbf{Q}$$

- **Nonlinear Barometric Observation Model**:
  Using local ground reference pressure $P_0$ (Option A):
  $$h(z) = P_0 \left(1.0 - \frac{z}{44330.0}\right)^{5.255} \quad [\text{Pa}]$$
  Analytical Jacobian:
  $$H_0 = \frac{\partial h}{\partial z} = -\frac{5.255 \, P_0}{44330.0} \left(1.0 - \frac{z}{44330.0}\right)^{4.255} \quad [\text{Pa/m}], \qquad \mathbf{H} = \begin{bmatrix} H_0 & 0 & 0 \end{bmatrix}$$

- **Scalar Measurement Update**:
  $$y = P_{\text{meas}} - h(z), \qquad S = H_0^2 P_{00} + R_{\text{baro}}$$
  $$\mathbf{K} = \frac{H_0}{S} \begin{bmatrix} P_{00} \\ P_{10} \\ P_{20} \end{bmatrix}$$
  $$\mathbf{x} \leftarrow \mathbf{x} + \mathbf{K} y, \qquad \mathbf{P} \leftarrow (\mathbf{I}_{3 \times 3} - \mathbf{K}\mathbf{H})\mathbf{P}$$

---

## 4. Empirical Noise Covariance Grounding (`LOG_0032.BIN`)
From offline statistical processing of flight blackbox dataset `LOG_0032.BIN` ($1519$ steady-state BMP180 samples at $50\,\text{Hz}$):
- $\sigma_P = 4.2221\,\text{Pa} \implies \sigma_P^2 = 17.8263\,\text{Pa}^2$
- Configured measurement noise: $R_{\text{baro}} = 17.8263 + 7.1737 = 25.0\,\text{Pa}^2$ (including $7.17\,\text{Pa}^2$ aerodynamic propwash margin)
- Process noise covariance matrix:
  $$\mathbf{Q} = \text{diag}\left(10^{-4}\,\text{m}^2,\, 10^{-3}\,(\text{m/s})^2,\, 10^{-6}\,(\text{m/s}^2)^2\right)$$

---

## 5. RTOS Task Topology & Inter-Process Communication
1. **Centralized Task Parameters (`include/config.h`)**:
   - `TASK_EKF_ALTITUDE_NAME = "EKF_ALTITUDE"`
   - `TASK_EKF_ALTITUDE_ID` in `enum TaskIDs`
   - `TASK_EKF_ALTITUDE_CORE = tskNO_AFFINITY`
   - `TASK_EKF_ALTITUDE_PRIORITY = 2` (Medium priority, Core math domain)
   - `TASK_EKF_ALTITUDE_FREQ_HZ = 100` (10 ms cadence)
   - `TASK_EKF_ALTITUDE_STACK_SIZE = 4096`
2. **Layer 3 Middleware IPC (`src/middleware/ipc.h`, `ipc.cpp`)**:
   - `RingBuffer<Vec3<float>, RING_BUF_SIZE> alt_state;` (`[z, v_z, a_z_inertial]`)
   - `RingBuffer<uint32_t, RING_BUF_SIZE> alt_timestamp;`
   - Thread safety guaranteed via `ekf_state_mutex`.
3. **Ground Reference Calibration Lifecycle (Option A)**:
   - **Active Calibration (`task_calib_baro`)**: Averages 150 pressure samples ($3\,\text{s}$ at $50\,\text{Hz}$), saves `baro_ref_pa` to Flash NVS, and updates `g_alt_ekf.set_reference_pressure(p0)`.
   - **Pre-Flight Launch Pad Readiness (`handle_state_ready`)**: Captures local stationary ground pressure upon entering `READY` state, dynamically resetting altitude state to $0.0\,\text{m}$ AGL right before takeoff.

---

## 6. Verification & Traceability
- **Mathematical Replay (`verify_altitude_ekf.py`)**: Replayed 15,404 samples ($30.81\,\text{s}$) of real flight blackbox data from `LOG_0032.BIN`. Verified $24.15\%$ peak-to-peak variance reduction over raw hypsometric baro altitude, zero velocity drift ($\bar{v}_z = -0.0177\,\text{m/s}$), and residual accelerometer bias estimation ($\bar{b}_{a,z} = 0.0108\,\text{m/s}^2$).
- **File Modifications**:
  - `include/config.h`: Added task enums and EKF altitude noise constants.
  - `src/middleware/ipc.h`, `ipc.cpp`: Added `baro_ref_pa` to `SensorCalibrationData` and declared `alt_state`, `alt_timestamp`.
  - `src/middleware/sysview_tracing.cpp`: Registered SystemView trace marker.
  - `src/core/sensor/calib.cpp`: Added NVS default reset and bounds validation.
  - `src/core/sensor/calib_tasks.cpp`: Completed `task_calib_baro` averaging and NVS save.
  - `src/core/sensor/sensor_preprocess_tasks.cpp`: Seeded `g_alt_ekf` from NVS on boot.
  - `src/core/math/estimators.h`, `estimators.cpp`: Implemented `AltitudeEKF` class.
  - `src/core/math/estimator_tasks.h`, `estimator_tasks.cpp`: Implemented `ekf_altitude_task` (100 Hz).
  - `src/app/app.cpp`: Spawned `ekf_altitude_task`.
  - `src/app/fsm.cpp`: Integrated live ground pressure calibration in `handle_state_ready()` and state transition.
