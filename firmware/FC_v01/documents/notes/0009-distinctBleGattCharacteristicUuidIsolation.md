---
id: "0009"
title: Distinct BLE GATT Characteristic UUID Isolation per Telemetry & OTA Stream Decision
author: Drone Research Team / Developer
date: 2026-09-30
time: 16:58:00 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#ble", "#telemetry", "#ota", "#gatt", "#uuid"]
references:
  - "[[0000-firmwareStructureLayerDecision]]"
  - "[[0001-taskConfigMiddlewareIsrDecision]]"
  - "[[0006-taskCreationConfigEnumsAndTaskArgsPassingDecision]]"
  - "[[0008-lpfAndCalibrationTaskArchitecture]]"
  - "[[0010-bleMinimumPacketFrequencyAndDualTriggerFlush]]"
  - "[[0011-variableBleTelemetryParsingAndZeroHandshakeDesign]]"
  - "[[0015-bleOtaArchitectureHardeningAndResourceCleanup]]"
  - "[[0016-coreDirectoryCompactionAndDomainConsolidation]]"
---

# Zettel 0009: Distinct BLE GATT Characteristic UUID Isolation per Telemetry & OTA Stream Decision

## 1. Metadata & Context
- **ID**: `0009`
- **Date**: 2026-09-30 (Updated: 2026-10-05)
- **Status**: Approved & Enforced
- **References**: `[Note 0000]`, `[Note 0001]`, `[Note 0006]`, `[Note 0008]`, `[Note 0010]`, `[Note 0011]`, `[Note 0015]`

---

## 2. Core Architectural Decision

### Strict 1:1 Task-to-Characteristic Isolation
To eliminate packet collisions, multi-sensor multiplexing bottlenecks, parsing ambiguity, and radio channel stalls across both high-frequency telemetry and wireless OTA firmware updating:

1. **Centralized UUID Service & Characteristic Series (`include/config.h`)**:
   Instead of multiplexing heterogeneous telemetry streams across shared generic pipes (`p_vec3_raw_char`), the architecture enforces dedicated 128-bit GATT characteristics:
   - **Telemetry GATT Service**: `BLE_SERVICE_TELEM_UUID` (`7c9a41b2-e2d7-4d9e-99f6-1e35a90d81f2`)
     - `BLE_CHAR_GYRO_RAW_UUID` / `BLE_CHAR_GYRO_FILT_UUID` (16-byte `BleGyroPacket`, 31 samples/packet $\approx$ 496B)
     - `BLE_CHAR_ACCEL_RAW_UUID` / `BLE_CHAR_ACCEL_FILT_UUID` (16-byte `BleAccelPacket`, 31 samples/packet $\approx$ 496B)
     - `BLE_CHAR_MAG_RAW_UUID` / `BLE_CHAR_MAG_FILT_UUID` (16-byte `BleMagPacket`, 31 samples/packet $\approx$ 496B)
     - `BLE_CHAR_BARO_RAW_UUID` / `BLE_CHAR_BARO_FILT_UUID` (8-byte `BleBaroPacket`, 63 samples/packet $\approx$ 504B)
     - `BLE_CHAR_ATT_QUAT_UUID` (20-byte `BleAttQuatPacket`, 25 samples/packet $\approx$ 500B)
     - `BLE_CHAR_ATT_EULER_UUID` (16-byte `BleAttEulerPacket`, 31 samples/packet $\approx$ 496B)
   - **OTA GATT Service**: `BLE_SERVICE_OTA_UUID` (`23408888-1f40-4cd8-9b89-ca8d45f8a5b0`)
     - `BLE_CHAR_OTA_CONTROL_UUID`: Handshake, progress, status, and abort signaling.
     - `BLE_CHAR_OTA_DATA_UUID`: High-throughput binary image streaming chunk sink.

2. **Single Source-of-Truth (SSoT) Bitmask Compilation (`ACTIVE_BLE_TELEM_MASK`)**:
   - Telemetry streams are individually conditionally instantiated in Layer 2 (`src/drivers/telemetry/ble_driver.cpp`) and spawned in Layer 5 (`src/app/app.cpp`) using `ACTIVE_BLE_TELEM_MASK` in `include/config.h`.
   - Disabled streams instantiate zero GATT characteristics and spawn zero FreeRTOS tasks, guaranteeing zero CPU, RAM, or radio bandwidth overhead when unused.

3. **Driver Isolation & Explicit Notification APIs (`src/drivers/telemetry/`)**:
   - Each stream possesses a dedicated driver notification function:
     - `send_ble_gyro_raw_notification()` $\rightarrow$ `p_gyro_raw_char`
     - `send_ble_accel_raw_notification()` $\rightarrow$ `p_accel_raw_char`
     - `send_ble_mag_raw_notification()` $\rightarrow$ `p_mag_raw_char`
     - `send_ble_baro_raw_notification()` $\rightarrow$ `p_baro_raw_char`
     - `send_ble_att_quat_notification()` $\rightarrow$ `p_att_quat_char`
     - `send_ble_att_euler_notification()` $\rightarrow$ `p_att_euler_char`
     - `send_ble_ota_response()` $\rightarrow$ `p_ota_control_char`

4. **Dedicated Task Mapping (`src/core/telemetry/ble_telem_tasks.cpp`)**:
   - Each active telemetry stream executes on its own deterministic FreeRTOS task runner (`ble_gyro_raw_trans_task`, `ble_accel_raw_trans_task`, `ble_mag_raw_trans_task`, etc.) pinned to its configured rate and stack size in `include/config.h`.
   - All tasks transmit exclusively via their assigned driver notification API without cross-talk or lock contention.

