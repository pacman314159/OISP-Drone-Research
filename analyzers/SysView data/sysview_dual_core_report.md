# SEGGER SystemView ESP32-S3 Dual-Core Trace Analysis Report

- **Core 0 Trace**: `core0.csv` (650 events)
- **Core 1 Trace**: `core1.csv` (203 events)
- **Trace Duration**: 0.0260 s (26.03 ms)
- **Total Combined Events**: 853

## 1. Dual-Core Task Timing & Jitter Statistics

| Task (Context) | Canonical Function | Priority | Affinity | Loops (C0/C1) | Freq (Hz) | Period (µs) | Jitter σ (µs) | Avg Run (µs) | WCET (µs) |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| ACCEL_GYRO_DAQ | accel_gyro_daq_task | 3 | CPU 0 Only | 11 (11/0) | 498.6 | 2005.7 | 51.2 | 720.6 | 934.0 |
| MAG_DAQ | mag_daq_task | 2 | CPU 0 Only | 2 (2/0) | 100.0 | 10000.0 | 0.0 | 1583.0 | 1583.0 |
| BLE_VEC3_TRANS_ | ble_vec3_trans_task | 1 | CPU 0 Only | 2 (2/0) | 100.0 | 10000.0 | 0.0 | 14.5 | 15.0 |
| btController | btController (NimBLE) | 23 | CPU 0 Only | 1 (0/0) | 100.0 | 10000.0 | 0.0 | 104.0 | 104.0 |

## 2. Dual-CPU Core Utilization & Load Breakdown

| Task / Entity | Core 0 Runtime (µs) | Core 0 Load (%) | Core 1 Runtime (µs) | Core 1 Load (%) | Aggregate SoC Load (%) | Free Stack (B) |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| ACCEL_GYRO_DAQ | 8513.0 | 32.70% | 0.0 | 0.00% | 16.35% | 2552 |
| MAG_DAQ | 7055.0 | 27.10% | 0.0 | 0.00% | 13.55% | 2564 |
| BLE_VEC3_TRANS_ | 137.0 | 0.53% | 0.0 | 0.00% | 0.26% | 6576 |
| btController | 104.0 | 0.40% | 0.0 | 0.00% | 0.20% | 1748 |
| **Total Tasks** | 15809.0 | 60.73% | 0.0 | 0.00% | 30.37% | - |
| **Hardware ISRs** | 0.0 | 0.00% | 0.0 | 0.00% | 0.00% | - |
| **Total Active CPU** | 15809.0 | 60.73% | 0.0 | 0.00% | 30.37% | - |
| **Idle Headroom** | 10221.0 | 39.27% | 26030.0 | 100.00% | 69.63% | - |

## 3. Hardware Interrupt (ISR) Breakdown per Core

### Core 0 ISRs

| ISR Name | Invocations | Min (µs) | Avg (µs) | Max (µs) | Core 0 Load (%) |
| :--- | :--- | :--- | :--- | :--- | :--- |

### Core 1 ISRs

| ISR Name | Invocations | Min (µs) | Avg (µs) | Max (µs) | Core 1 Load (%) |
| :--- | :--- | :--- | :--- | :--- | :--- |

## 4. Primary Tasks Forensic Analysis

### `accel_gyro_daq_task`
- **Role**: Primary Real-Time 500 Hz IMU Sampling Loop
- **Predefined Target in config.h**: 500.0 Hz (2000.0 µs period)
- **Affinity**: Core 0 exclusively (11 loops)
- **Overall Session Rate**: 498.6 Hz (Avg Period: 2005.7 µs, σ: 51.2 µs)
- **Steady-State Synchronized Rate**: **494.3 Hz** (Avg Period: **2023.0 µs**, Jitter σ: **39.9 µs**) -> Matches 500 Hz target within 1.7%!
- **Startup Catch-Up Burst Rate**: 501.5 Hz (Avg Period: 1994.2 µs, 6 loops)
- **Execution**: Avg Runtime: 720.6 µs | WCET: 934.0 µs
- **CPU Load**: Core 0: 32.70% (Sampling Window Load: 37.63%)

### `mag_daq_task`
- **Role**: Magnetometer Data Acquisition
- **Affinity**: Core 0 exclusively (2 loops)
- **Measured Rate**: 100.0 Hz (Avg Period: 10000.0 µs)
- **Jitter (σ)**: 0.0 µs
- **Execution**: Avg Runtime: 1583.0 µs | WCET: 1583.0 µs
- **CPU Load**: Core 0: 27.10% | Core 1: 0.00%

### `ble_vec3_trans_task`
- **Role**: High-speed BLE GATT Vector Telemetry Transmitter
- **Affinity**: Cross-Core Migrating (`tskNO_AFFINITY`)
- **Loop Distribution**: Core 0: 2 loops | Core 1: 0 loops (Total: 2 loops)
- **Session Overall Rate**: 100.0 Hz (Avg Period: 10000.0 µs, σ: 0.0 µs)
- **Steady-State Rate (Core 0)**: 100.0 Hz (Period: 10000.0 µs, σ: 0.0 µs)
- **Execution**: Avg Runtime: 14.5 µs | WCET: 15.0 µs
- **CPU Load**: Core 0: 0.53% | Core 1: 0.00% | Combined SoC: 0.26%
