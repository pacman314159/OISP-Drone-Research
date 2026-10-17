---
id: "0028"
title: INA226 Power Module I2C1 DAQ, Shunt Resistor Calibration, Millis Scale Metrics & Battery Health Monitoring Architecture
author: Drone Research Team / Developer
date: 2026-10-15
time: 15:35:00 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#power", "#ina226", "#i2c1", "#battery", "#telemetry", "#blackbox"]
references:
  - "[[0000-firmwareStructureLayerDecision]]"
  - "[[0001-taskConfigMiddlewareIsrDecision]]"
  - "[[0002-taskEnumsAndHalAbstractionDecision]]"
  - "[[0006-taskCreationConfigEnumsAndTaskArgsPassingDecision]]"
  - "[[0009-distinctBleGattCharacteristicUuidIsolation]]"
  - "[[0018-sdCardLoggingRestorationAndUserButtonShutdown]]"
  - "[[0019-universalFlightDataAnalyzerAndBinaryTelemetryAlignmentSpecification]]"
  - "[[0020-fsmBackbonePreFlightSensorChecksAndActiveCalibrationArchitecture]]"
  - "[[0027-preflightSensorDataStreamCheckFsmArchitecture]]"
  - "[[0029-unificationOfInterTaskRingBufferSizeConstant]]"
  - "[[0031-sixGatePreflightTerminalDevelopmentAndTaskEncapsulationArchitecture]]"
---

# Zettel 0028: INA226 Power Module I2C1 DAQ, Shunt Resistor Calibration, Millis Scale Metrics & Battery Health Monitoring Architecture

## 1. Metadata & Scope
- **ID**: `0028`
- **Author**: Drone Research Team / Developer
- **Date**: 2026-10-15
- **Time**: 15:35:00 +07:00
- **Status**: Approved & Implemented
- **References**: `[Note 0000]`, `[Note 0001]`, `[Note 0002]`, `[Note 0006]`, `[Note 0009]`, `[Note 0018]`, `[Note 0019]`, `[Note 0020]`, `[Note 0027]`

---

## 2. Problem Statement & Motivation
Accurate, real-time electrical monitoring of the entire quadcopter power distribution system (4 BLDC motors + 4in1 ESC + Flight Controller + Peripherals) is crucial for:
1. **Preventing Mid-Flight Battery Collapse**: Unmonitored lithium battery cell exhaustion under high-throttle load leads to severe voltage sag, ESC brownouts, and catastrophic loss of control.
2. **Dynamic ESC & Motor Power Profiling**: Characterizing aerodynamic thrust-to-power efficiency ($g/\text{W}$) and peak stall currents during rapid maneuvers requires high-speed synchronized current and power sampling.
3. **Safe Pre-Flight Arming Gating**: Permitting flight initialization on an empty, damaged, or deeply discharged LiPo pack must be prevented deterministically before motors spin up.

To satisfy these requirements, a dedicated Texas Instruments INA226 high-side/low-side current and power monitor was integrated with an ultra-low-resistance $0.5\,\text{m}\Omega$ ($0.0005\,\Omega$) series shunt resistor.

---

## 3. Mathematical Calibration & Shunt Resistor Parameterization

### A. Shunt Resistor Sizing & Precision
- **Shunt Resistance**: $R_{\text{shunt}} = 0.0005\,\Omega$ ($0.5\,\text{m}\Omega$).
- **Maximum Expected Continuous Current**: Quadcopter hovering + forward flight consumes $\approx 20\text{--}60\,\text{A}$. Peak full-throttle burst reaches $\approx 100\text{--}140\,\text{A}$.
- **Shunt Voltage Full-Scale**: The INA226 internal programmable gain amplifier (PGA) accommodates a maximum differential shunt input voltage of $\pm 81.92\,\text{mV}$.
- **Maximum Theoretical Current**:
  $$I_{\text{max\_shunt}} = \frac{V_{\text{shunt\_fs}}}{R_{\text{shunt}}} = \frac{81.92\,\text{mV}}{0.5\,\text{m}\Omega} = 163.84\,\text{A}$$

### B. Current & Power LSB Scaling (Enforced Millis Scale)
To guarantee integer precision, eliminate floating-point truncation, and satisfy the system-wide requirement for **millis scale metrics** (`voltage_mv`, `current_ma`, `power_mw`, `energy_mah`):
- **Current LSB Selection**:
  $$\text{Current\_LSB} = 5.0\,\text{mA} = 0.005\,\text{A}$$
- **Full-Scale Dynamic Range**:
  $$I_{\text{max\_measurable}} = \text{Current\_LSB} \times 2^{15} = 5.0\,\text{mA} \times 32768 = 163.84\,\text{A}$$
- **Calibration Register ($CAL$) Derivation**:
  According to TI INA226 datasheet equation:
  $$CAL = \text{trunc}\left(\frac{0.00512}{\text{Current\_LSB} \times R_{\text{shunt}}}\right) = \text{trunc}\left(\frac{0.00512}{0.005 \times 0.0005}\right) = \text{trunc}\left(\frac{0.00512}{0.0000025}\right) = 2048$$
  > [!TIP]
  > Because $0.00512 / 0.0000025$ equals exactly $2048.0$, the mathematical truncation error is strictly **$0.000\%$**.
- **Power LSB**:
  $$\text{Power\_LSB} = 25 \times \text{Current\_LSB} = 25 \times 5.0\,\text{mA} = 125.0\,\text{mW}$$
- **Bus Voltage LSB**: Fixed hardware internal scaling of $1.25\,\text{mV}/\text{LSB}$.
- **Shunt Voltage LSB**: Fixed hardware internal scaling of $2.5\,\mu\text{V}/\text{LSB}$.

---

## 4. Hardware Bus Architecture & JTAG Conflict Resolution

### A. Bus Allocation
The primary I2C0 bus on GPIO 21 (SDA) / 22 (SCL) hosts high-frequency primary sensors (MPU6050 IMU, QMC5883L Magnetometer, BMP180 Barometer). To prevent transaction congestion and bandwidth saturation:
- The INA226 is routed to the secondary **I2C1** bus on **GPIO 19 (SCL)** and **GPIO 20 (SDA)**.
- `INA226_I2C_ADDR = 0x40` (A0 = GND, A1 = GND).

### B. JTAG / USB Serial Pin Conflict Gating
On ESP32-S3, GPIO 19 (USB_D-) and GPIO 20 (USB_D+) multiplex with internal USB-JTAG tracing.
- Centralized toggle in `include/config.h`:
  ```cpp
  #define DISABLE_I2C1_FOR_JTAG 0 // Set to 0 to enable I2C1 power monitoring
  ```
- When `DISABLE_I2C1_FOR_JTAG = 0`, GPIO 19/20 are claimed by hardware `i2c1`. Flashing and serial console communication continue normally over UART COM7 (`CP2102`). Native hardware OpenOCD tracing cannot execute concurrently while I2C1 power monitoring is active.

---

## 5. System-Wide Layered Architectural Realization

```
+-----------------------------------------------------------------------------------+
| LAYER 5: APPLICATION & FSM (src/app/)                                             |
|  - Preflight Check 7: check_battery_condition() (Cell Count & Low Voltage Latch)  |
|  - Check 1 Stream Health: Monitors power_data (freshness <= 200ms, range 1-36V)   |
|  - Visual Error Indication: Slow Red LED Blink on EVT_BATTERY_LOW                 |
+-----------------------------------------------------------------------------------+
                                         ^
                                         | Reads RingBuffer
+-----------------------------------------------------------------------------------+
| LAYER 4: CORE TASKS (src/core/)                                                   |
|  - sensor/daq_tasks.cpp: power_daq_task (50 Hz, Anti-Windup, sysview, Energy mAh) |
|  - telemetry/ble_telem_tasks.cpp: BlePowerPacket (20B packed, UUID 3e5a7b1c...)   |
|  - blackbox/sd_logger_task.cpp: PktPower (24B 4-byte aligned, LOG_MASK_POWER)    |
+-----------------------------------------------------------------------------------+
                                         ^
                                         | Pushes Measurements
+-----------------------------------------------------------------------------------+
| LAYER 3: MIDDLEWARE IPC (src/middleware/)                                         |
|  - ipc.h / ipc.cpp: struct PowerMeasurement (voltage_mv, current_ma, power_mw...) |
|  - RingBuffer<PowerMeasurement, 32> power_data                                    |
|  - RingBuffer<uint32_t, 32> power_timestamp                                       |
|  - Mutex: i2c1_mutex for thread-safe bus transactions                             |
+-----------------------------------------------------------------------------------+
                                         ^
                                         | Driver Read
+-----------------------------------------------------------------------------------+
| LAYER 2: DRIVERS (src/drivers/power/)                                             |
|  - ina226.h / ina226.cpp: Stateless Driver over HAL_I2C_BUS&                      |
|  - Big-Endian 16-bit registers, TI ID check (0x5449, 0x2260), Config 0x4327      |
+-----------------------------------------------------------------------------------+
                                         ^
                                         | I2C Transactions
+-----------------------------------------------------------------------------------+
| LAYER 1: HARDWARE ABSTRACTION LAYER (HAL)                                         |
|  - HAL_I2C_BUS (i2c1: GPIO 19 SCL, GPIO 20 SDA @ 400 kHz)                        |
+-----------------------------------------------------------------------------------+
```

### A. Layer 2: Stateless Driver (`INA226`)
- Follows the target-agnostic design (`HAL_I2C_BUS&`).
- Self-test verifies Manufacturer ID (`0x5449` = "TI") and Die ID (`0x2260`).
- Configures averaging to 4 samples, $1.1\,\text{ms}$ bus and shunt conversion times (`0x4327`), resulting in total conversion time $\approx 8.8\,\text{ms}$ (easily exceeding the $50\,\text{Hz}$ / $20\,\text{ms}$ DAQ loop requirement).

### B. Layer 3: Middleware IPC Data Structures
```cpp
struct PowerMeasurement {
  float voltage_mv; // Millivolts (mV)
  float current_ma; // Milliamperes (mA)
  float power_mw;   // Milliwatts (mW)
  float energy_mah; // Cumulative energy consumed (mAh)
};
```

### C. Layer 4: DAQ Task, Energy Integration & Hot-Plug Auto-Recovery
- `power_daq_task(void* arg)` runs at $50\,\text{Hz}$ (`TASK_POWER_DAQ_ID = 10`).
- Integrates energy consumption in real time:
  $$\Delta t_{\text{hours}} = \frac{\Delta t_{\mu\text{s}}}{3.6 \times 10^9},\quad E_{\text{mAh}} += I_{\text{mA}} \times \Delta t_{\text{hours}}$$
- Enforces anti-windup overrun guard:
  ```cpp
  if((now - last_wake_time) > (period_ticks * 2)) last_wake_time = now;
  ```
- **Hot-Plug Transient Auto-Recovery**:
  Plugging in the LiPo battery post-boot produces high-inrush spark transients across the power distribution board that can momentarily desynchronize the I2C bus or trigger an INA226 power-on reset. `power_daq_task` tracks consecutive read failures:
  - If $\ge 5$ consecutive failures occur ($\approx 100\,\text{ms}$), `ina->init()` is automatically invoked.
  - `ina->init()` re-initializes the underlying physical I2C bus peripheral (`_i2c_bus.init(I2C_FREQ_HZ)`), re-verifies the TI chip ID (`0x2260`), restores continuous sampling configuration (`0x4327`), and re-writes the calibration register (`CAL = 2048`), restoring communication transparently without requiring a manual board restart.

### D. Layer 5: FSM Pre-Flight Safety & LiPo Cell Auto-Detection
1. **Dynamic Cell Detection**: Auto-detects pack configuration ($1\text{S}\text{--}8\text{S}$) based on nominal $3800\,\text{mV}/\text{cell}$:
   $$\text{cell\_count} = \text{clamp}\left(\text{round}\left(\frac{V_{\text{pack\_mv}}}{3800.0}\right), 1, 8\right)$$
2. **Strict Arming Gating & Terminal Development Failover**:
   - `REQUIRE_BATTERY_FOR_ARMING` has been completely eliminated; battery presence and health are mandatory.
   - If average cell voltage $V_{\text{cell}} < 3300\,\text{mV}$ (`BATTERY_CELL_MIN_VOLTAGE_MV`), reading is invalid, or no battery data arrives within `SENSOR_STREAM_STARTUP_TIMEOUT_MS` (2.5 s):
     - Sets `EVT_BATTERY_LOW` and clears `EVT_BATTERY_VOLTAGE_OK`.
     - Fails Gate 5 and triggers immediate, one-way transition to **`STATE_DEVELOPMENT`** (`Rule_Preflight_To_Development`, see `[[0031]]`).
     - Motors are permanently locked (`PWM = 0`) with solid amber alert LED until cold reboot.
3. **Stream Health Integration**: Added as part of Gate 5 in `check_power_initial_condition()`. Verifies timestamp freshness ($\Delta t \le 200\,000\,\mu\text{s}$) and voltage sanity ($1000\,\text{mV} \le V_{\text{pack}} \le 36000\,\text{mV}$).

---

## 6. Offline Analysis Engine (`universal_flight_analyzer.py`)

1. **Protocol Integration**:
   - `PKT_TYPE_POWER = 0x09`, packet size 24 bytes (1B tag + 23B payload `<I3xffff`).
2. **Analysis Menu Expansion**:
   - Option `[17]`: Power Histogram & Standard Deviation (Voltage, Current, Power).
   - Option `[18]`: Power Time Series (Voltage, Current, Power, Energy vs. Time).
   - Option `[19]`: All Time Series Plots.
   - Option `[22]`: Complete Analysis.
3. **Automated Electrical Diagnostics**:
   - Initial pack voltage, minimum loaded voltage, maximum voltage sag ($\Delta V$).
   - Average current, peak burst current ($I_{\text{peak}}$).
   - Average power, peak electrical wattage ($P_{\text{peak}}$).
   - Total energy extracted ($E_{\text{mAh}}$) across flight duration.

---

## 7. Traceability Matrix

| File | Layer | Changes Made |
|---|---|---|
| `include/config.h` | Layer 1/Config | Added INA226 hardware constants, calibration value `2048`, task config, BLE UUID, packet mask `0x09`. |
| `src/drivers/power/ina226.h` | Layer 2 | Created stateless driver interface and `PowerSample` structure. |
| `src/drivers/power/ina226.cpp` | Layer 2 | Implemented 16-bit register handling, TI chip ID check, configuration, calibration. |
| `src/drivers/telemetry/ble_packet.h` | Layer 2 | Added packed 20-byte `BlePowerPacket` structure. |
| `src/drivers/telemetry/ble_driver.h/.cpp` | Layer 2 | Registered Power GATT characteristic (`3e5a7b1c-...`) and notification handler. |
| `src/middleware/ipc.h/.cpp` | Layer 3 | Declared and instantiated `power_data` and `power_timestamp` ring buffers. |
| `src/core/sensor/daq_tasks.h/.cpp` | Layer 4 | Implemented `power_daq_task` with anti-windup, energy accumulation, SystemView hooks. |
| `src/core/telemetry/ble_telem_tasks.cpp` | Layer 4 | Added Power telemetry stream to `ble_telem_task`. |
| `src/core/blackbox/sd_logger_task.cpp` | Layer 4 | Added 4-byte aligned `PktPower` logging for `LOG_MASK_POWER`. |
| `src/app/app.cpp` | Layer 5 | Instantiated I2C1 bus and INA226 driver; spawned `power_daq_task`. |
| `src/app/fsm.h/.cpp` | Layer 5 | Added battery preflight checks, cell count auto-detection, low-voltage visual latch, 6-stream health. |
| `analyzers/universal_flight_analyzer.py` | Tooling | Added `PKT_TYPE_POWER` parser, plotting routines (HIST, TIME), and electrical metrics summary. |

---

## 8. Cross-References & Traceability
- [[0000-firmwareStructureLayerDecision]]: Layer 2 driver and Layer 4 sensor task boundaries.
- [[0001-taskConfigMiddlewareIsrDecision]]: Centralized `i2c1_mutex` protection rule.
- [[0002-taskEnumsAndHalAbstractionDecision]]: Target-agnostic HAL I2C bus abstraction.
- [[0006-taskCreationConfigEnumsAndTaskArgsPassingDecision]]: Centralized config enums and driver pointer arguments.
- [[0009-distinctBleGattCharacteristicUuidIsolation]]: Dedicated GATT characteristic UUID allocation.
- [[0018-sdCardLoggingRestorationAndUserButtonShutdown]]: SD card sector serialization format.
- [[0019-universalFlightDataAnalyzerAndBinaryTelemetryAlignmentSpecification]]: Binary flight data parser and analyzer alignment.
- [[0020-fsmBackbonePreFlightSensorChecksAndActiveCalibrationArchitecture]]: FSM state machine transitions and safety arming gates.
- [[0027-preflightSensorDataStreamCheckFsmArchitecture]]: 6-stream health verification including power telemetry.
- [[0029-unificationOfInterTaskRingBufferSizeConstant]]: Unification of power ring buffer capacity under RING_BUF_SIZE.
