# 🚁 OISP Drone Research — Exhaustive Technical Master Report

> **Project Title**: OISP Drone Research (00-drone-research-2026)  
> **Target Hardware Platform**: Custom Quadcopter UAV (ESP32-S3 WROOM-1 Dual-Core @ 240 MHz running FreeRTOS / ESP-IDF)  
> **Core Engineering Philosophy**: Strictly data-driven engineering backed by physical dynamics modeling, discrete-time control theory, empirical sensor FFT spectral analysis, and hard real-time RTOS schedulability constraints.

---

## 📑 Detailed Table of Contents
1. [System Architecture & Repository Structure](#1-system-architecture--repository-structure)
2. [Mechanical Engineering & Product Data Management (PDM)](#2-mechanical-engineering--product-data-management-pdm)
3. [Electrical Engineering & PCB Hardware Architecture](#3-electrical-engineering--pcb-hardware-architecture)
4. [Real-Time Firmware & RTOS Architecture (Target System Architecture v01 Spec)](#4-real-time-firmware--rtos-architecture-target-system-architecture-v01-spec)
5. [Extended Kalman Filter (EKF) Sensor Fusion & Matrix Math](#5-extended-kalman-filter-ekf-sensor-fusion--matrix-math)
6. [Control Systems Theory & Flight Architecture](#6-control-systems-theory--flight-architecture)
7. [Digital Signal Processing & Python Analyzer Suite](#7-digital-signal-processing--python-analyzer-suite)
8. [Ground Telemetry Station (FCDatMon — Status & Architecture)](#8-ground-telemetry-station-fcdatmon--status--architecture)
9. [Research Foundation & Literature Synchronization](#9-research-foundation--literature-synchronization)

---

## 1. System Architecture & Repository Structure

> ℹ️ **Firmware Development Status**: The overall integrated flight control system has **not been fully built or merged into a single unified application yet**. The directories inside `firmware/` currently represent **independent modular testing blocks and standalone subsystem prototypes** (e.g. individual sensor drivers, EKF fusion tests, BLE DAQ, LoRa transceivers, thrust testbench) developed independently by the team to validate hardware peripherals and core algorithms prior to full flight stack integration.

```
00-drone-research-2026/
├── .agents/                        # Team roster, system rules & agent definitions
│   ├── AGENTS.md                   # Core system architecture & security guidelines
│   └── TEAM_ROSTER.md              # 8-person specialized subagent team handles & roles
├── 3d_models/                      # Autodesk Inventor CAD models & Git LFS binaries
│   ├── Full_body/                  # Full drone assembly (SYS-ASM-0000-Main.iam)
│   └── Flight Controller/          # PCB & component 3D step models
├── pcb/                            # KiCad v10 PCB schematics & multi-layer layouts
├── firmware/                       # Independent Modular Testing Blocks (FreeRTOS / ESP-IDF)
│   ├── EKF-sensor-fusion/          # Standalone EKF state estimation & sensor driver test block
│   │   ├── src/main.cpp            # EKF test application loop & task definitions
│   │   ├── test/EKF.cpp            # EKF prediction & decoupled update math routines
│   │   └── src/sensor/             # GY87_MPU6050 and GY87_HMC5883L I2C drivers
│   ├── BaroSensor/                 # Standalone BMP180 pressure & altitude test block
│   ├── ble_9dof_daq/               # Standalone 9-DOF BLE telemetry streaming test block
│   ├── RF_module/                  # Standalone Ra-02 LoRa (433MHz) & NRF24L01+ test block
│   ├── thrust_testbench/           # Standalone BLDC dynamic response & PWM thrust testbench
│   └── sensor/                     # Raw sensor driver validation module
├── analyzers/                      # Python Signal Processing & Mathematical Tools
│   ├── baro_fft.py                 # RFFT power spectral density of barometer data
│   ├── imu_fft.py                  # 6-axis IMU FFT noise spectral analysis
│   ├── mag_fft.py                  # 3-axis Magnetometer FFT noise spectral analysis
│   ├── baro_distribution.py        # Barometer OSS (0 vs 3) probability density plots
│   └── imu_distribution.py         # Accelerometer/Gyroscope Gaussian noise fits
├── FCDatMon/                       # Python DearPyGui Ground Telemetry App (Prototype)
│   ├── core/                       # Async BLE/Serial readers, state & data managers
│   └── main.py                     # Immediate-mode GPU rendering application
├── system_architecture/            # Architectural flowcharts & PDF specification
│   └── System Architecture v01.pdf # Target flight controller architecture specification
└── research_docs/                  # Academic papers on RTOS scheduling & control stability
```

---

## 2. Mechanical Engineering & Product Data Management (PDM)

### Airframe Specifications & Physical Parameters
* **Wheelbase**: 450 mm diagonal motor-to-motor distance (F450 quadcopter geometry).
* **Structural Frame**: High-strength nylon composite arms (red front, white rear for orientation reference), rigid central FR4 structural plates, dedicated avionics deck, and lower $3\text{S}$ LiPo battery sled.
* **Propulsion System**: 4x BLDC **A2217/8T - 1100KV** motors paired with $10 \times 4.5$ inch 2-blade propellers and **40A** Electronic Speed Controllers (ESCs).

### ASME Y14.100 / MIL-STD-31000B PDM Numbering Standard
To prevent long Windows path failures (`MAX_PATH < 260`) and maintain strict version control across Autodesk Inventor assemblies linked via Git LFS, the project enforces a semi-smart indentured naming convention:

$$\text{[PRJ]}-\text{[SYS]}-\text{[TYP]}-\text{[ID]}\_\text{[DESCRIPTION]}\_\text{R[REV]}.\text{[EXT]}$$

* **`[PRJ]`**: `OISP` (Project Code Name)
* **`[SYS]`**: System Identifier (`SYS`: Top System, `MEC`: Mechanical Airframe, `PWR`: Power, `PRP`: Propulsion, `AVI`: Avionics/Sensors, `PCB`: Printed Circuit Board)
* **`[TYP]`**: File Type (`ASM`: Assembly `.iam`, `PRT`: Part `.ipt`)
* **`[ID]`**: Indentured Assembly Level:
  * `0000`: Level 0 Master Assembly (`OISP-SYS-ASM-0000_Quadcopter_Top_R01.iam`)
  * `1000`: Level 1 Airframe Sub-Assembly (`OISP-MEC-ASM-1000_F450_Frame_R01.iam`)
  * `2000`: Level 1 Propulsion Sub-Assembly (`OISP-PRP-ASM-2000_Propulsion_Sys_R01.iam`)
  * `2100`: Level 2 Motor/ESC Sub-Assembly (`OISP-PRP-ASM-2100_Motor_ESC_Pair_R01.iam`)
  * `2101`: Level 3 Component Part (`OISP-PRP-PRT-2101_BLDC_2212_920KV_R01.ipt`)
  * `4000`: Level 1 Flight Controller PCB (`OISP-PCB-ASM-4000_Flight_Controller_R01.iam`)

### Mass Properties & Inertia Tensor Matrix
By strictly linking CAD mass properties across all sub-assemblies, the system computes the exact center of gravity ($\text{CG}$) and full 3D moment of inertia tensor matrix needed directly by the state-space flight dynamics model:

$$\mathbf{I} = \begin{bmatrix} I_{xx} & -I_{xy} & -I_{xz} \\ -I_{yx} & I_{yy} & -I_{yz} \\ -I_{zx} & -I_{zy} & I_{zz} \end{bmatrix}$$

---

## 3. Electrical Engineering & PCB Hardware Architecture

### Master Flight Controller Hardware Specifications
1. **Core Microcontroller**: **ESP32-S3 WROOM-1**
   * CPU: Dual-Core Xtensa 32-bit LX7 @ 240 MHz.
   * Floating-Point Unit: Hardware Single-Precision FPU.
   * Memory: 512 KB SRAM internal DRAM, 8 MB SPI PSRAM, 16 MB QSPI NOR Flash (W25Q128JV).
   * Peripherals: Hardware I2C controllers (`I2C0`, `I2C1`), SPI with DMA, UARTs, RMT hardware peripheral, LEDC/MCPWM timers.
2. **Integrated GY-87 10-DOF Sensor Array**:
   * **MPU-6050**: 6-Axis MEMS MotionTracking (3-axis gyroscope $\pm 2000^\circ/\text{s}$, 3-axis accelerometer $\pm 8\text{g}$).
   * **HMC5883L**: 3-Axis Digital Magnetometer ($\pm 8\text{ Gauss}$) with dedicated `HMC5883L DRDY` hardware interrupt line.
   * **BMP180**: Barometric Pressure Sensor ($300-1100\text{ hPa}$, $0.03\text{ hPa}$ resolution).
3. **RF & Communications Array**:
   * FlySky FS-iA6B Receiver (PPM frame ingestion via hardware ESP32 RMT peripheral @ $44\text{ Hz}$).
   * Ra-02 LoRa Module (SX1278 @ $433\text{ MHz}$) / NRF24L01+ ($2.4\text{ GHz}$).
   * Integrated ESP32 Bluetooth Low Energy (BLE 5.0) telemetry pipeline.

### KiCad v10 PCB Layout & Power Isolation Rules
* **Star Grounding Topology**: Heavy motor switching return currents ($160\text{A}$ total peak draw across four $40\text{A}$ ESCs) create significant ground bounce. The KiCad v10 PCB routes high-current ground planes separately from sensitive $3.3\text{V}$ MEMS analog and digital ground planes, joining them strictly at a single quiet star-ground point.
* **Dual Linear LDO Regulation & Schottky Isolation**:
  * **Digital Rail**: Dedicated LDO regulating power for ESP32-S3 core, QSPI Flash, and LoRa/NRF RF power amplifiers.
  * **Analog Sensor Rail**: Secondary low-noise LDO feeding MEMS sensors (MPU-6050, HMC5883L, BMP180), isolated via BAT54C Schottky diodes to suppress voltage ripple caused by RF transmission bursts.
* **XT60 Mechanical Strain Relief**: Copper polygon pours surrounding the high-current XT60 battery mounting pads are reinforced with thermal relief via-stitching to prevent mechanical castellation delamination during plug insertion/extraction.

---

## 4. Real-Time Firmware & RTOS Architecture (Target System Architecture v01 Spec)

> 💡 **Design Blueprint**: This section outlines the **target architecture specification** defined in `system_architecture/System Architecture v01.pdf` towards which the independent firmware test blocks are being integrated.

### Sensor DAQ Pipeline & Interrupt Signal Conventions
1. **MPU-6050 (Gyro & Accel DAQ @ 500 Hz)**:
   * **Non-Interrupt Polling**: The primary IMU is **not** driven by a hardware interrupt. An RTOS task (`Accel Gyro DAQ`) periodically polls raw 6-axis data over `I2C0` (guarded by `i2c0_mutex`).
   * **Ring Buffer & Filtering**: Raw samples (`gx_raw, ax_raw, ...`) are stored in an internal **Ring buffer** (saving newest index). A calibration & filter task (`Accel Gyro Calib & Filter @ 250 Hz`) copies data via `memcpy()` under `taskENTER_CRITICAL()` to produce filtered `ax, ay, az` and `gx, gy, gz`.
2. **HMC5883L Magnetometer DRDY ISR**:
   * **Hardware ISR Connection**: The `HMC5883L DRDY` hardware pin triggers a dedicated Hardware Interrupt Service Routine (Red Ellipse ISR) calling `setDataReady()`, which flags/wakes the `Mag DAQ` task.
   * `Mag DAQ` reads `I2C0` under `i2c0_mutex` guard and streams raw magnetometer data into a **Ring buffer** for the `Mag Calib & Filter` task to output `mx, my, mz`.
3. **Barometer DAQ (`Baro DAQ`)**:
   * Periodically polls `BMP180` pressure data over `I2C0` under `i2c0_mutex` guard into a **Ring buffer** for `Baro Calib & Filter` to output `pressure`.
4. **Flysky Receiver & RMT Decoder Task (@ 44 Hz)**:
   * FS-iA6B receiver frames stream into SoC `RMT Memory` hardware, passed via `xQueueSend()` to the `RX Decoder` task to decode roll, pitch, yaw, and throttle stick commands.

### Control Loops & Target Execution Rates
* **EKF Estimation Tasks (@ 100 Hz)**:
  * `EKF Roll & Pitch (100 Hz)`: Consumes `ax, ay, az` and `gx, gy, gz` under `taskENTER_CRITICAL()`.
  * `EKF Yaw (100 Hz)`: Consumes `mx, my, mz` under `taskENTER_CRITICAL()`.
  * `Altitude & Vz Estimator (100 Hz)`: Consumes `pressure` and vertical acceleration.
  * Updates shared `state_vector` protected by `taskENTER_CRITICAL()`.
* **Attitude Outer Loop (@ 100 Hz)**:
  * Consumes stick references (`pitch_ref, roll_ref, vz_ref`) and `state_vector` under `taskENTER_CRITICAL()`, outputting target rates (`pitch_rate_ref, roll_rate_ref, yaw_rate_ref`).
* **Attitude Inner Loop (@ 250 Hz, Priority 4)**:
  * High-priority task consuming target rates and gyro states, generating control torques (`pitch_torque, roll_torque`).
* **Motor Mixer & PWM Generator (@ 250 Hz)**:
  * Computes `duty1, duty2, duty3, duty4` for the SoC hardware `PWM Generator` driving 4 ESCs (`ESC1`, `ESC2`, `ESC3`, `ESC4`).
* **Dual I2C Bus Partitioning**:
  * `I2C0` (Guarded by `i2c0_mutex`): MPU-6050, HMC5883L, BMP180.
  * `I2C1` (Guarded by `i2c1_mutex`): Power Monitor Module (`50 Hz`).

---

## 5. Extended Kalman Filter (EKF) Sensor Fusion & Matrix Math

### State Vector Definition ($7 \times 1$)
The EKF tracks a 7-element state vector consisting of attitude quaternions and gyroscope angular rate biases:

$$\mathbf{x} = \begin{bmatrix} q_0 & q_1 & q_2 & q_3 & b_x & b_y & b_z \end{bmatrix}^T$$

where $\mathbf{q} = [q_0, q_1, q_2, q_3]^T$ represents the orientation quaternion, and $\mathbf{b} = [b_x, b_y, b_z]^T$ represents gyro bias in $\text{rad/s}$.

### Kinematic Prediction Step
Predicts states forward using discrete integration timestep $\Delta t$:

1. **Unbiased Gyroscope Rate**:
   $$\omega_x = (\omega_{raw,x} \cdot 0.0174533) - b_x, \quad \omega_y = (\omega_{raw,y} \cdot 0.0174533) - b_y, \quad \omega_z = (\omega_{raw,z} \cdot 0.0174533) - b_z$$

2. **Quaternion Numerical Integration**:
   $$\begin{bmatrix} q_0 \\ q_1 \\ q_2 \\ q_3 \end{bmatrix}_{k|k-1} = \begin{bmatrix} q_0 - \frac{\Delta t}{2}(\omega_x q_1 + \omega_y q_2 + \omega_z q_3) \\ q_1 + \frac{\Delta t}{2}(\omega_x q_0 - \omega_y q_3 + \omega_z q_2) \\ q_2 + \frac{\Delta t}{2}(\omega_x q_3 + \omega_y q_0 - \omega_z q_1) \\ q_3 - \frac{\Delta t}{2}(\omega_x q_2 - \omega_y q_1 - \omega_z q_0) \end{bmatrix}$$

3. **Linearized Process Jacobian Matrix ($\mathbf{F}_{7 \times 7}$)**:
   $$\mathbf{F} = \begin{bmatrix} 1 & -\frac{\Delta t}{2}\omega_x & -\frac{\Delta t}{2}\omega_y & -\frac{\Delta t}{2}\omega_z & \frac{\Delta t}{2}q_1 & \frac{\Delta t}{2}q_2 & \frac{\Delta t}{2}q_3 \\ \frac{\Delta t}{2}\omega_x & 1 & \frac{\Delta t}{2}\omega_z & -\frac{\Delta t}{2}\omega_y & -\frac{\Delta t}{2}q_0 & \frac{\Delta t}{2}q_3 & -\frac{\Delta t}{2}q_2 \\ \frac{\Delta t}{2}\omega_y & -\frac{\Delta t}{2}\omega_z & 1 & \frac{\Delta t}{2}\omega_x & -\frac{\Delta t}{2}q_3 & -\frac{\Delta t}{2}q_0 & \frac{\Delta t}{2}q_1 \\ \frac{\Delta t}{2}\omega_z & \frac{\Delta t}{2}\omega_y & -\frac{\Delta t}{2}\omega_x & 1 & \frac{\Delta t}{2}q_2 & -\frac{\Delta t}{2}q_1 & -\frac{\Delta t}{2}q_0 \\ 0 & 0 & 0 & 0 & 1 & 0 & 0 \\ 0 & 0 & 0 & 0 & 0 & 1 & 0 \\ 0 & 0 & 0 & 0 & 0 & 0 & 1 \end{bmatrix}$$

4. **Covariance Propagation**:
   $$\mathbf{P}_{k|k-1} = \mathbf{F} \mathbf{P}_{k-1|k-1} \mathbf{F}^T + \mathbf{Q}$$

---

### Decoupled Observation Updates ($3 \times 3$ Matrix Optimization)

To eliminate the computational burden of inverting a large $6 \times 6$ or $7 \times 7$ innovation matrix on an embedded microcontroller, the EKF intentionally splits observation updates into separate 3D vector update steps:

#### A. Accelerometer Measurement Update Step
Expected gravity vector in body frame:

$$\mathbf{h}_a(\mathbf{q}) = \begin{bmatrix} 2(q_1 q_3 - q_0 q_2) \\ 2(q_0 q_1 + q_2 q_3) \\ q_0^2 - q_1^2 - q_2^2 + q_3^2 \end{bmatrix}$$

Measurement Jacobian $\mathbf{H}_a$ ($3 \times 7$ matrix):

$$\mathbf{H}_a = \begin{bmatrix} -2q_2 & 2q_3 & -2q_0 & 2q_1 & 0 & 0 & 0 \\ 2q_1 & 2q_0 & 2q_3 & 2q_2 & 0 & 0 & 0 \\ 2q_0 & -2q_1 & -2q_2 & 2q_3 & 0 & 0 & 0 \end{bmatrix}$$

Innovation Covariance Matrix $\mathbf{S}_a$ ($3 \times 3$ matrix) and Kalman Gain $\mathbf{K}_a$ ($7 \times 3$ matrix):

$$\mathbf{S}_a = \mathbf{H}_a \mathbf{P} \mathbf{H}_a^T + \mathbf{R}_a$$

$$\mathbf{K}_a = \mathbf{P} \mathbf{H}_a^T \mathbf{S}_a^{-1} \quad \text{(using optimized } \mathbf{S}_a.\text{inverse3x3() routine)}$$

$$\mathbf{x} \leftarrow \mathbf{x} + \mathbf{K}_a (\mathbf{z}_a - \mathbf{h}_a), \quad \mathbf{P} \leftarrow (\mathbf{I}_{7 \times 7} - \mathbf{K}_a \mathbf{H}_a) \mathbf{P}$$

#### B. Magnetometer Measurement Update Step ($75\text{ Hz}$)
Expected earth magnetic field vector in body frame:

$$\mathbf{h}_m(\mathbf{q}) = \begin{bmatrix} b_x(q_0^2 + q_1^2 - q_2^2 - q_3^2) + 2 b_z(q_1 q_3 - q_0 q_2) \\ 2 b_x(q_1 q_2 - q_0 q_3) + 2 b_z(q_0 q_1 + q_2 q_3) \\ 2 b_x(q_0 q_2 + q_1 q_3) + b_z(q_0^2 - q_1^2 - q_2^2 + q_3^2) \end{bmatrix}$$

Measurement Jacobian $\mathbf{H}_m$ ($3 \times 7$ matrix):

$$\mathbf{H}_m = \begin{bmatrix} 2b_x q_0 - 2b_z q_2 & 2b_x q_1 + 2b_z q_3 & -2b_x q_2 - 2b_z q_0 & -2b_x q_3 + 2b_z q_1 & 0 & 0 & 0 \\ -2b_x q_3 + 2b_z q_1 & 2b_x q_2 + 2b_z q_0 & 2b_x q_1 + 2b_z q_3 & -2b_x q_0 + 2b_z q_2 & 0 & 0 & 0 \\ 2b_x q_2 + 2b_z q_0 & 2b_x q_3 - 2b_z q_1 & 2b_x q_0 - 2b_z q_2 & 2b_x q_1 + 2b_z q_3 & 0 & 0 & 0 \end{bmatrix}$$

Kalman Gain Calculation using $3 \times 3$ Inversion:

$$\mathbf{S}_m = \mathbf{H}_m \mathbf{P} \mathbf{H}_m^T + \mathbf{R}_m, \quad \mathbf{K}_m = \mathbf{P} \mathbf{H}_m^T \mathbf{S}_m^{-1}$$

$$\mathbf{x} \leftarrow \mathbf{x} + \mathbf{K}_m (\mathbf{z}_m - \mathbf{h}_m), \quad \mathbf{P} \leftarrow (\mathbf{I}_{7 \times 7} - \mathbf{K}_m \mathbf{H}_m) \mathbf{P}$$

---

## 6. Control Systems Theory & Flight Architecture

### Cascaded PID Architecture

```
 Pilot Setpoints      +   Attitude PID   Desired Rates  +   Angular Rate PID   Motor Commands
(Angle Roll/Pitch) --->(O)--->[Outer Loop]------------->(O)--->[Inner Loop 250Hz]----> Motor Mixer
                        ^ -                              ^ -
                        |                                |
                   EKF Quaternion                  MPU-6050 Gyro
```

1. **Outer Loop (Attitude / Angle Loop @ 100 Hz)**: Computes attitude orientation error using quaternions, outputting desired angular rates (`pitch_rate_ref, roll_rate_ref, yaw_rate_ref`).
2. **Inner Loop (Rate Loop @ 250 Hz, Priority 4)**: Consumes target rates and raw gyro data to generate control torque demands (`pitch_torque, roll_torque`), passed to the Motor Mixer for computing motor PWM duty cycles (`duty1..duty4`).

---

## 7. Digital Signal Processing & Python Analyzer Suite

### Barometer Spectral Analysis (`baro_fft.py`)
Processes pressure telemetry data (`pressure_data_time_oss3.txt`) to evaluate atmospheric noise and oversampling effects:
* Converts millisecond timestamps to seconds, dynamically calculating sample frequency $f_s = \frac{1}{\Delta t}$.
* Detrends baseline DC offset ($P - \bar{P}$) and executes Real Fast Fourier Transform (`np.fft.rfft`).
* Normalizes amplitude spectrum to Pascals ($\text{dB} = 20 \log_{10}(\text{Amp})$) to design discrete low-pass pressure filters.

---

## 8. Ground Telemetry Station (FCDatMon — Status & Architecture)

> ⚠️ **Status Note**: `FCDatMon` represents a prototype telemetry visualizer in the repository, but is **not under active consideration for the current phase**. Primary engineering efforts remain concentrated on firmware RTOS scheduling, hardware PCB signal integrity, sensor fusion math, and Python analyzers.

### System Architecture Summary
* **UI Framework**: Python 3.11+ using **DearPyGui (DPG)** for GPU immediate-mode rendering.
* **Concurrency Pattern**: Producer-Consumer decoupling via Python `threading.Thread` and `queue.Queue`.
* **Transport Drivers**: `bleak` for Bluetooth LE communication and `pyserial` for USB UART data streams.

---

## 9. Research Foundation & Literature Synchronization

The repository directly integrates formulas, timing bounds, and structural designs from peer-reviewed literature stored in `/research_docs`:

1. **RTOS Schedulability vs. Closed-Loop Stability**:
   * *"Closing the gap between Stability and Schedulability paper.pdf"* & *"LIPIcs.ECRTS.2024.17.pdf"*: Establishes upper bounds on task latency jitter ($\tau_{delay} < 2\text{ ms}$) to ensure phase margin non-degradation in cascaded PID loops.
2. **Physics-Based Thruster Dynamics**:
   * *"Toward_an_Accurate_Physics-Based_UAV_Thruster_Model.pdf"*: Models BLDC motor lag as a first-order low-pass transfer function $G_{motor}(s) = \frac{1}{\tau_m s + 1}$.
3. **Open-Source Autopilot Benchmarks**:
   * Crazyflie C firmware RTOS structure analysis and PX4 Autopilot architecture studies for modular driver abstraction.

---

> **Summary**: The **OISP Drone Research** codebase is an end-to-end multi-disciplinary engineering project uniting Autodesk Inventor CAD/PDM, KiCad v10 star-grounded PCB layout, discrete control theory, decoupled $3 \times 3$ EKF sensor fusion, and Python signal processing analyzers. The sub-folders in `firmware/` currently contain independent modular driver/sub-system test blocks towards full integration.
