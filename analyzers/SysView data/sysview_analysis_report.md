# SEGGER SystemView Session Analysis Report

- **Source File**: `core0.csv`
- **Trace Duration**: 0.5636 s (563.59 ms)
- **Total Events**: 4,328

## 1. Task Full Loop Timing Statistics

| Task | Priority | Loops | Freq (Hz) | Period (µs) | Jitter (µs) | Avg Iteration (µs) | WCET (µs) | CPU Load (%) | Free Stack (Bytes) |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| ACCEL_GYRO_DAQ | 3 | 50 | 1006.5 | 993.5 | 43.2 | 772.8 | 974.0 | 4.81% | 2376 |
| MAG_DAQ | 2 | 4 | 48.0 | 20816.5 | 197.5 | 20768.5 | 20966.0 | 0.69% | 2500 |
| btController | 23 | 6 | 24.7 | 40537.4 | 77169.1 | 94.7 | 109.0 | 0.10% | 1748 |
| BLE_VEC3_TRANS_ | 1 | 1 | 0.0 | 0.0 | 0.0 | 59.0 | 59.0 | 0.01% | 6544 |

## 2. Hardware Interrupts (ISRs)

| ISR Name | Invocations | Min (µs) | Avg (µs) | Max (µs) | CPU Load (%) |
| :--- | :--- | :--- | :--- | :--- | :--- |
| SYSTIMER_TARGET0 | 55 | 32.0 | 15732.8 | 203954.0 | 36.29% |
| SysTick | 563 | 11.0 | 361.9 | 197215.0 | 36.15% |
| I2C_EXT0 | 312 | 1.0 | 33.5 | 292.0 | 0.92% |
| FROM_CPU_INTR0 | 213 | 9.0 | 10.3 | 15.0 | 0.39% |

## 3. Custom Instrumentation Markers

| Marker | Context | Passes | Min (µs) | Avg (µs) | Max (µs) |
| :--- | :--- | :--- | :--- | :--- | :--- |
| Stop Marker 0x00000001 | ACCEL_GYRO_DAQ | 52 | 0.0 | 4089.5 | 174017.0 |
| Stop Marker 0x00000004 | MAG_DAQ | 3 | 20571.0 | 73607.7 | 179286.0 |
| Stop Marker 0x00000005 | BLE_VEC3_TRANS_ | 1 | 13.0 | 13.0 | 13.0 |
