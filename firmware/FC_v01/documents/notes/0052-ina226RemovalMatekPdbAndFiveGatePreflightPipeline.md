---
id: "0052"
title: Complete INA226 Decommissioning, Matek PDB XT60 Transition & Five-Gate Pre-Flight Safety Pipeline
author: Drone Research Team / Antigravity
date: 2026-11-14
time: 19:28:00 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#power", "#ina226", "#pdb", "#fsm", "#preflight", "#safety"]
references:
  - "[[0000-firmwareStructureLayerDecision]]"
  - "[[0002-taskEnumsAndHalAbstractionDecision]]"
  - "[[0006-taskCreationConfigEnumsAndTaskArgsPassingDecision]]"
  - "[[0020-fsmBackbonePreFlightSensorChecksAndActiveCalibrationArchitecture]]"
  - "[[0027-preflightSensorDataStreamCheckFsmArchitecture]]"
  - "[[0028-ina226PowerModuleI2c1DaqAndBatteryMonitoringArchitecture]]"
  - "[[0031-sixGatePreflightTerminalDevelopmentAndTaskEncapsulationArchitecture]]"
---

# Zettel 0052: Complete INA226 Decommissioning, Matek PDB XT60 Transition & Five-Gate Pre-Flight Safety Pipeline

## 1. Metadata & Scope
- **ID**: `0052`
- **Author**: Drone Research Team / Antigravity
- **Date**: 2026-11-14
- **Time**: 19:28:00 +07:00
- **Status**: Approved & Implemented
- **Supersedes**: `[[0028-ina226PowerModuleI2c1DaqAndBatteryMonitoringArchitecture]]`; amends `[[0031-sixGatePreflightTerminalDevelopmentAndTaskEncapsulationArchitecture]]` (Gate 5 retirement)
- **Scope**: Top-to-bottom removal of `INA226` power monitoring across Layers 1–5 (`include/config.h`, `src/drivers/`, `src/middleware/`, `src/core/`, `src/app/`).

---

## 2. Context & Motivation

The hardware platform transitioned from the legacy custom I2C1 `INA226` shunt monitor (`0x40`) to a passive **Matek PDB XT60** power distribution board with integrated 5V/12V linear/buck BECs powered by a 3S LiPo pack (`Ovonic 2300mAh 3S 45C`). Because the Matek PDB provides zero digital I2C voltage/current telemetry, maintaining a compile-time bypass (`ENABLE_EXTERNAL_POWER_MONITOR = 0`) left dead event bitmasks, FSM transition blockers, Layer 3 IPC header coupling (`ipc.h` unconditionally including `ina226.h`), BSS ring buffers (`power_data`, `power_timestamp`, `s_ch_power`), BLE GATT UUID entries, and SD logger packet definitions across all five architectural layers.

To maintain structural cleanliness before introducing new external sensors (`MTF-01P`, GNSS) and hardware adaptation patterns, all `INA226` infrastructure was permanently ripped out from Layer 5 down to Layer 1.

---

## 3. Architectural Decision

1. **Five-Gate Pre-Flight Pipeline**: Collapse the Six-Gate sequential pipeline (`[Note 0031]`) to five mandatory safety gates inside `handle_state_preflight()`:
   - **Gate 1/5**: `check_sensor_data_stream()` (IMU, Magnetometer, Barometer stream freshness & physical range bounds)
   - **Gate 2/5**: `check_platform_stationary()`
   - **Gate 3/5**: `check_passive_calibration()`
   - **Gate 4/5**: `check_platform_tilt()`
   - **Gate 5/5**: `check_rc_controller_signal()` (formerly Gate 6/6)
2. **FSM Event & Rule Pruning (`src/app/fsm.h/.cpp`)**: Delete `EVT_BATTERY_VOLTAGE_OK` and `EVT_BATTERY_LOW` from `SystemEventMasks`, and remove them from `Rule_Preflight_To_Ready`, `Rule_Preflight_To_Development`, and `Rule_Ready_To_Takeoff`.
3. **Full Multi-Layer Purging (`src/core/`, `src/middleware/`, `src/drivers/`, `include/config.h`)**:
   - Delete `check_power_initial_condition()`, `power_daq_task()`, `s_ch_power`, `PktPower`, `trk_power`, `power_data`, `power_timestamp`, `p_power_char`, `BlePowerPacket`, and `src/drivers/power/ina226.h/.cpp`.
   - Remove `TASK_POWER_DAQ_*` across all task enums, `ENABLE_EXTERNAL_POWER_MONITOR`, `INA226_*`, `SENSOR_STREAM_POWER_TIMEOUT_US`, `BATTERY_CELL_*_VOLTAGE_MV`, `BLE_POWER_SAMPLES_PER_PACKET`, `BLE_CHAR_POWER_UUID`, `BLE_TELEM_MASK_POWER`, `PKT_TYPE_POWER`, and `LOG_MASK_POWER` in `include/config.h`.
4. **Protocol & Analyzer Backward Compatibility**: Preserve explicit bit positions and numeric IDs for downstream telemetry/logging enumerations (`PKT_TYPE_RC = 0x0A`, `PKT_TYPE_ALT = 0x0B`, `BLE_TELEM_MASK_RC_CHANNELS = (1 << 11)`, `BLE_TELEM_MASK_ALT = (1 << 12)`, `LOG_MASK_RC = (1 << 9)`, `LOG_MASK_ALT = (1 << 10)`) and leave `analyzers/universal_flight_analyzer.py` untouched so historical SD logs (`0000-LOG.txt`–`0002-LOG.txt`) remain parseable.

---

## 4. Impacted Files Across 5 Layers

| Layer | File | Action |
|---|---|---|
| Layer 5 (App/FSM) | `src/app/fsm.h`, `src/app/fsm.cpp`, `src/app/app.cpp` | Removed battery event flags, gate evaluation, `INA226 ina(i2c1)`, `ina.init()`, and `power_daq_task` spawn |
| Layer 4 (Core) | `src/core/safety/preflight_checks.h/.cpp` | Removed `check_power_initial_condition()` and renumbered gates `[GATE 1/5]`–`[GATE 5/5]` |
| Layer 4 (Core) | `src/core/sensor/daq_tasks.h/.cpp` | Removed `power_daq_task()` declaration and implementation |
| Layer 4 (Core) | `src/core/telemetry/ble_telem_tasks.cpp`, `src/core/blackbox/sd_logger_task.cpp` | Removed `s_ch_power` (~500 B BSS), `PktPower`, `trk_power`, and power stream serialization |
| Layer 3 (IPC/Trace) | `src/middleware/ipc.h/.cpp`, `src/middleware/sysview_tracing.cpp` | Removed `ina226.h` include, `power_data`, `power_timestamp`, and `TASK_POWER_DAQ_ID` marker |
| Layer 2 (Drivers) | `src/drivers/power/ina226.h/.cpp` | Deleted standalone `INA226` driver files and empty `src/drivers/power/` directory |
| Layer 2 (Drivers) | `src/drivers/telemetry/ble_driver.cpp`, `src/drivers/telemetry/ble_packet.h` | Removed `p_power_char` GATT characteristic, switch case, and `BlePowerPacket` |
| Layer 1 / Config | `include/config.h` | Purged all power/INA226 constants, task enum entries, UUIDs, and thresholds |

---

## 5. Verification

- Static symbol audit across `src/` and `include/` (`rg -n -i` across 25 power/INA226/battery/6-gate symbols) returned **0 matches**.
- Knowledge graph synchronized via `graphify update .` and Zettelkasten links updated in `[[0028-ina226PowerModuleI2c1DaqAndBatteryMonitoringArchitecture]]`, `[[0031-sixGatePreflightTerminalDevelopmentAndTaskEncapsulationArchitecture]]`, and `documents/Firmware Structure.md`.
- Note: Per `.agents/AGENTS.md` build policy, `pio run -e esp32-s3-devkitc-1 -j 4` is reserved for manual developer verification.

---

## 6. Cross-References & Traceability
- [[0000-firmwareStructureLayerDecision]]: 5-layer isolation rules enforced during cleanup.
- [[0020-fsmBackbonePreFlightSensorChecksAndActiveCalibrationArchitecture]]: FSM backbone and pre-flight state supervision.
- [[0027-preflightSensorDataStreamCheckFsmArchitecture]]: Gate 1 sensor stream verification without power stream dependency.
- [[0028-ina226PowerModuleI2c1DaqAndBatteryMonitoringArchitecture]]: Superseded INA226 power monitoring design.
- [[0031-sixGatePreflightTerminalDevelopmentAndTaskEncapsulationArchitecture]]: Original 6-gate pipeline reduced to 5 gates.
