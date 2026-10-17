---
id: "0017"
title: BMP180 Non-Blocking Two-Phase I2C DAQ Architecture, Pipelined Conversion & Vertical Velocity Fusion Specification
author: Drone Research Team / Developer
date: 2026-10-07
time: 17:48:00 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#baro", "#bmp180", "#i2c", "#freertos", "#dsp", "#kalman"]
references:
  - "[[0000-firmwareStructureLayerDecision]]"
  - "[[0001-taskConfigMiddlewareIsrDecision]]"
  - "[[0002-taskEnumsAndHalAbstractionDecision]]"
  - "[[0006-taskCreationConfigEnumsAndTaskArgsPassingDecision]]"
  - "[[0007-systemViewTracingAndRTOSTimingAnalysisConventions]]"
  - "[[0008-lpfAndCalibrationTaskArchitecture]]"
  - "[[0016-coreDirectoryCompactionAndDomainConsolidation]]"
  - "[[0022-unifiedPressureRingBufferSensorVariableConsolidationAndAttitudeTaskRemoval]]"
  - "[[0029-unificationOfInterTaskRingBufferSizeConstant]]"
---

# Zettel 0017: BMP180 Non-Blocking Two-Phase I2C DAQ Architecture, Pipelined Conversion & Vertical Velocity Fusion Specification

## 1. Metadata & Context
- **ID**: `0017`
- **Author**: Drone Research Team / Developer
- **Date**: 2026-10-07
- **Time**: 17:48:00 +07:00
- **Status**: Approved & Enforced
- **References**: `[Note 0000]`, `[Note 0001]`, `[Note 0002]`, `[Note 0006]`, `[Note 0007]`, `[Note 0008]`, `[Note 0016]`

---

## 2. Core Architectural Decisions

### A. Non-Blocking Two-Phase I2C Transaction Model
Unlike continuous streaming sensors (e.g. MPU6050, HMC5883L), the BMP180 barometric pressure sensor requires a strict two-stage transaction:
1. **Initiate Phase**: Command write to register `0xF4` (`CTRL_MEAS`, `0x2E` for temperature, `0x34 + (oss << 6)` for pressure).
2. **Wait Gap**: Internal ADC conversion time ($t_{conv} \in [4.5, 25.5]\text{ ms}$).
3. **Retrieve Phase**: Read conversion result from data registers `0xF6`–`0xF8`.

Holding the shared I2C bus mutex (`i2c0_mutex`) across this conversion delay is strictly forbidden. Holding the mutex for $> 4.5\text{ ms}$ starves the 500 Hz IMU flight loop (`BUS_MUTEX_TIMEOUT_MS = 20\text{ ms}`), precipitating missed loop deadlines and sensor buffer corruption. Mutex acquisition MUST be strictly isolated to the transmission burst itself ($< 200\ \mu\text{s}$ per transaction).

### B. Pipelined Inter-Loop Conversion (Zero Intra-Loop Yield)
To prevent internal task yield fragmentation, eliminate CPU polling overhead, and preserve strict SystemView timing integrity (`[Note 0007]`), `baro_daq_task` implements **Pipelined Inter-Loop Conversion**:
- In iteration $k$, the task reads the converted sample triggered in iteration $k-1$ from register `0xF6`, pushes it to `pressure_raw`, triggers the conversion for iteration $k+1$ in register `0xF4`, and releases `i2c0_mutex`.
- Total mutex hold duration per iteration is $< 350\ \mu\text{s}$.
- The task then enters `vTaskDelayUntil(&last_wake_time, period_ticks)`. The physical ADC conversion occurs transparently in the background while the FreeRTOS task is blocked, granting 100% CPU to the primary flight control and telemetry loops.

### C. Oversampling Setting (OSS) & Timing Feasibility
The internal oversampling setting $OSS \in \{0, 1, 2, 3\}$ averages $N = 2^{OSS}$ samples internally, scaling root-mean-square noise by $\sigma_{RMS} = \sigma_0 \cdot 2^{-OSS/2}$:
- $\text{OSS} = 0$ (Ultra Low Power): $1$ sample, $t_{conv} \le 4.5\text{ ms}$, noise $0.06\text{ hPa}$ ($0.50\text{ m}$).
- $\text{OSS} = 1$ (Standard): $2$ samples, $t_{conv} \le 7.5\text{ ms}$, noise $0.05\text{ hPa}$ ($0.41\text{ m}$).
- $\text{OSS} = 2$ (High Resolution): $4$ samples, $t_{conv} \le 13.5\text{ ms}$, noise $0.04\text{ hPa}$ ($0.33\text{ m}$).
- $\text{OSS} = 3$ (Ultra High Res): $8$ samples, $t_{conv} \le 25.5\text{ ms}$, noise $0.03\text{ hPa}$ ($0.25\text{ m}$).

**Frequency Rule**:
- At `TASK_BARO_DAQ_FREQ_HZ = 50` ($T_s = 20\text{ ms}$), the maximum achievable oversampling setting is **`BMP_OSS_HIGH_RES` (OSS = 2)**, ensuring $t_{conv} = 13.5\text{ ms} < 20\text{ ms}$ with $6.5\text{ ms}$ guard margin.
- If `BMP_OSS_ULTRA_HIGH_RES` (OSS = 3) is selected, loop frequency MUST be configured to $\le 30\text{ Hz}$ (e.g. `TASK_BARO_DAQ_FREQ_HZ = 25`, $T_s = 40\text{ ms}$) to accommodate $t_{conv} = 25.5\text{ ms}$.

### D. Interleaved Low-Frequency Temperature Refresh
Ambient temperature varies on the order of seconds. Ingesting temperature at 50 Hz degrades pressure throughput. `baro_daq_task` schedules a single temperature update once per second ($N = \text{TASK_BARO_DAQ_FREQ_HZ}$ cycles):
- In the refresh cycle, uncompensated temperature ($UT$) is read and cached to refresh compensation factor $B_5$ without calling intra-loop blocking delays.

### E. Elimination of Legacy Driver Deficiencies
1. **Hardware Abstraction**: Replaced direct `<Wire.h>` calls with `HAL_I2C_BUS&` per `[Note 0002]`.
2. **Integer Arithmetic Hardening**: The legacy `int16_t b5` overflow vulnerability is resolved by enforcing `int32_t b5` per Bosch BMP180 specification ($B_5 \in [-32768, 65535]$).
3. **Hardware Sanity Verification**: Mandatory chip ID validation (`0xD0 == 0x55`) and division-by-zero protection ($b_4 \neq 0$, $(x_1 + md) \neq 0$).

### F. Vertical Velocity ($v_z$) & Acceleration Drift Compensation Context
Barometric altitude differentiating ($\frac{dz}{dt}$) amplifies high-frequency noise proportional to frequency ($|j\omega| = \omega$), generating $\sigma_{v_z} > 15\text{ m/s}$ noise if used unassisted. Conversely, integrating vertical acceleration ($a_z$) from the 500 Hz IMU drifts rapidly due to sensor bias.
The raw pressure stream pushed to `pressure_raw` is structured specifically for Layer 4 state estimators (`src/core/math/estimator_tasks.cpp`):
- High-frequency 500 Hz IMU $a_z$ provides zero-lag dynamic response.
- 50 Hz barometric pressure provides drift-free reference to eliminate double-integration velocity and altitude drift.

### G. Barometer Raw BLE Telemetry Streaming (`ble_baro_raw_trans_task`)
Per `[Note 0009]` and `[Note 0010]`, raw barometric pressure data is streamed over Bluetooth Low Energy using a dedicated 128-bit GATT Characteristic:
- **Characteristic UUID**: `BLE_CHAR_BARO_RAW_UUID` = `"d4e7a2b9-3c81-4f5e-9a62-8b1e4c7d0f3a"`
- **Packet Structure**: Packed 8-byte `BleBaroPacket` (`uint32_t timestamp` [us], `float pressure` [Pa]).
- **Packet Aggregation**: `BLE_BARO_SAMPLES_PER_PACKET = 63` samples per MTU payload ($63 \times 8\text{B} = 504\text{B}$, comfortably inside the 512B MTU).
- **Dual-Trigger Flush**: Flushes immediately upon filling 63 samples or when `BLE_TELEM_MIN_PACKET_FREQ_HZ` (10 Hz) timeout expires to prevent packet stalling during low flight activity (`[Note 0010]`).

---

## 3. Impact & Architectural Constraints
1. **Zero Bus Starvation**: Driver code in Layer 2 NEVER includes `ipc.h` or calls `xSemaphoreTake()`; mutex bounding is managed strictly by task runners in Layer 4 (`daq_tasks.cpp`).
2. **Pipelined Loop Contract**: Periodic loop execution period $T_s$ must strictly exceed sensor conversion time $t_{conv}$.
3. **SystemView Trace Uniformity**: `SYSVIEW_START` and `SYSVIEW_END` encompass the full iteration execution, maintaining uniform microsecond duration without embedded sleep intervals.
4. **IPC Data Flow**: Data is injected atomically into `RingBuffer<float, RING_BUF_SIZE> pressure` and `RingBuffer<uint32_t, RING_BUF_SIZE> pressure_timestamp`.
5. **GATT Characteristic Isolation**: Raw barometric streaming is decoupled onto its dedicated UUID handle without multiplexing overhead.

---

## 4. Cross-References & Traceability
- [[0000-firmwareStructureLayerDecision]]: Layer 2 driver and Layer 4 sensor task boundaries.
- [[0001-taskConfigMiddlewareIsrDecision]]: Centralized `i2c0_mutex` protection rule.
- [[0002-taskEnumsAndHalAbstractionDecision]]: Target-agnostic HAL and deterministic delay rules.
- [[0006-taskCreationConfigEnumsAndTaskArgsPassingDecision]]: Centralized config enums and direct driver pointer argument passing.
- [[0007-systemViewTracingAndRTOSTimingAnalysisConventions]]: SystemView full-loop measurement and jitter standards.
- [[0008-lpfAndCalibrationTaskArchitecture]]: Downstream digital filter and estimator interface.
- [[0009-distinctBleGattCharacteristicUuidIsolation]]: Dedicated GATT characteristic UUID allocation.
- [[0010-bleMinimumPacketFrequencyAndDualTriggerFlush]]: Telemetry packet aggregation and dual-trigger timeout flushing.
- [[0011-variableBleTelemetryParsingAndZeroHandshakeDesign]]: Binary telemetry packet encoding.
- [[0016-coreDirectoryCompactionAndDomainConsolidation]]: Sensor DAQ consolidation in `src/core/sensor/`.
- [[0022-unifiedPressureRingBufferSensorVariableConsolidationAndAttitudeTaskRemoval]]: Unification of barometer pressure into a single ring buffer without discrete LPF.
- [[0029-unificationOfInterTaskRingBufferSizeConstant]]: Unification of IMU_RAW_RING_BUF_SIZE and IMU_FILT_RING_BUF_SIZE into RING_BUF_SIZE.
