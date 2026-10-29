---
id: "0049"
title: Firmware Architecture Audit, Refactoring Session Summary & Build Verification
author: Drone Research Team / Developer
date: 2026-10-28
time: 01:45:00 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#audit", "#refactoring", "#summary"]
references:
  - "[[0000-firmwareStructureLayerDecision]]"
  - "[[0040-fsmCalibrationTerminalStateAndRebootBoundaryDecision]]"
  - "[[0041-seggerSystemViewExclusionForSetupAndCalibrationTasks]]"
  - "[[0042-layer5ExclusiveTaskSuspensionAuthorityAndOtaDecoupling]]"
  - "[[0043-layer2DriverDecouplingFromLayer4MathAndDirectSimdScaling]]"
  - "[[0044-systemPhysicalParametersAndMassBudget]]"
  - "[[0045-rcReceiverCommandBlackboxSdCardLoggingArchitecture]]"
  - "[[0046-fsmDirectEventReportingAndControlledLayerInversionException]]"
  - "[[0047-layer1RmtPpmSyncGapBleedAndPulseRxDecoupling]]"
  - "[[0048-halSpiInterfaceSegregationAndSdCardDriverDecoupling]]"
---

# Zettel 0049: Firmware Architecture Audit, Refactoring Session Summary & Build Verification

## 1. Metadata & Scope
- **ID**: `0049`
- **Author**: Drone Research Team / Developer
- **Date**: 2026-10-28
- **Time**: 01:45:00 +07:00
- **Status**: Complete & Verified Sprint Summary
- **Module Context**: Full `firmware/FC_v01` codebase and `documents/notes/` knowledge vault

---

## 2. Executive Summary of Refactoring Sprint
This sprint resolved all high-priority architectural inversions, safety hazards, timing starvation, and documentation hygiene issues across `firmware/FC_v01`:

### 2.1 Layer Boundaries & Dependency Decoupling
1. **Layer 2 $\rightarrow$ Layer 4 Decoupling** (`[[0043-layer2DriverDecouplingFromLayer4MathAndDirectSimdScaling]]`):
   - Excised `matrix.h` and `Vec3` types from `MPU6050` and `HMC5883L`. Replaced with primitive C buffers (`float[3]`, `int16_t[3]`) and in-place `hal_simd::scale()` (`dsps_mulc_f32`).
2. **Layer 4 $\rightarrow$ Layer 5 Suspension Decoupling** (`[[0042-layer5ExclusiveTaskSuspensionAuthorityAndOtaDecoupling]]`):
   - Removed task suspension authority from `ble_ota_tasks.cpp`. Exclusive suspension authority restored to Layer 5 (`app.cpp`).
3. **Controlled FSM Event Dispatch Exception** (`[[0046-fsmDirectEventReportingAndControlledLayerInversionException]]`):
   - Formalized developer design decision permitting lower layers to report state machine events directly into `fsm_handle_event()` for zero-latency, deterministic safety transitions.
4. **Layer 1 SPI Interface Segregation** (`[[0048-halSpiInterfaceSegregationAndSdCardDriverDecoupling]]`):
   - Stripped FATFS volume mounting, card detection, and disk space methods from `HAL_SPI`. Encapsulated all storage volume lifecycle operations in Layer 2 `SDCardDriver`.

### 2.2 Safety, Real-Time Scheduling & Math Fixes
1. **Empty RingBuffer Guards**: Added `.empty()` verification prior to calling `.front()` on `accel_raw`, `gyro_raw`, `gyro`, `accel`, and `mag` in `src/core/safety/preflight_checks.cpp`.
2. **Quaternion Telemetry Constructor**: Added `BleAttQuatPacket(uint32_t, const Vec4&)` in `src/drivers/telemetry/ble_packet.h` preventing compile failures when `BLE_TELEM_MASK_ATT_QUAT` is enabled.
3. **Preflight Stationary Check Units**: Replaced $1.0\text{ g}$ scalar with `STANDARD_GRAVITY_MSS` ($9.80665\text{ m/s}^2$).
4. **RC Scheduling Starvation**: Reduced blocking queue timeout from 30 ms to 5 ms in `rc_receiver_task.cpp`.
5. **Blackbox Struct Alignment**: Added `_pad[3]` to `PktAttQuat` in `sd_logger_task.cpp` to prevent binary stream misalignment.
6. **FSM Calibration Loop Deadlock** (`[[0040-fsmCalibrationTerminalStateAndRebootBoundaryDecision]]`): Formalized CALIB as terminal state requiring hardware reboot.
7. **SystemView Tracing Exclusion** (`[[0041-seggerSystemViewExclusionForSetupAndCalibrationTasks]]`): Removed SystemView overhead from setup and calibration tasks.
8. **Mathematical Library Refactoring**: Modernized `src/core/math/lpf.h` with OOP encapsulation and removed `<Arduino.h>` dependency.

### 2.3 Knowledge Vault & Graphify Hygiene
1. **Duplicate Zettel IDs Resolved**: Renumbered mass budget note to `[[0044-systemPhysicalParametersAndMassBudget]]` and RC blackbox note to `[[0045-rcReceiverCommandBlackboxSdCardLoggingArchitecture]]`. Updated all cross-references across the vault.
2. **Broken Wikilinks Repaired**: Corrected slug targets in Note `0030`.
3. **Deferred Technical Debt**: Documented RMT PPM sync-gap bleed in `[[0047-layer1RmtPpmSyncGapBleedAndPulseRxDecoupling]]`.
4. **AST Knowledge Graph**: Graphify index synchronized across all modules.

---

## 3. Verification & Build Targets
- **Target 1**: `pio run -e fc-v01 -j 4` (Production flight controller firmware)
- **Target 2**: `pio run -e fc-v01-ota -j 4` (BLE OTA distribution binary)
