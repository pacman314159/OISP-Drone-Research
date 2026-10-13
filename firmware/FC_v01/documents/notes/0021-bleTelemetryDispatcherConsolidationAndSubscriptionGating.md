---
id: "0021"
title: BLE Telemetry Dispatcher Consolidation, Subscription-Aware Gating & FCDatMon Stride Specification
author: Drone Research Team / Developer
date: 2026-10-10
time: 20:20:00 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#ble", "#telemetry", "#fcdatmon", "#memory-optimization"]
references:
  - "[[0001-taskConfigMiddlewareIsrDecision]]"
  - "[[0002-taskEnumsAndHalAbstractionDecision]]"
  - "[[0006-taskCreationConfigEnumsAndTaskArgsPassingDecision]]"
  - "[[0009-distinctBleGattCharacteristicUuidIsolation]]"
  - "[[0010-bleMinimumPacketFrequencyAndDualTriggerFlush]]"
  - "[[0011-variableBleTelemetryParsingAndZeroHandshakeDesign]]"
  - "[[0016-coreDirectoryCompactionAndDomainConsolidation]]"
---

# Zettel 0021: BLE Telemetry Dispatcher Consolidation, Subscription-Aware Gating & FCDatMon Stride Specification

## 1. Metadata & Scope
- **ID**: `0021`
- **Author**: Drone Research Team / Developer
- **Date**: 2026-10-10
- **Time**: 20:20:00 +07:00
- **Status**: Approved & Implemented
- **References**: `[Note 0001]`, `[Note 0002]`, `[Note 0006]`, `[Note 0009]`, `[Note 0010]`, `[Note 0011]`, `[Note 0016]`

---

## 2. Problem Statement & Motivation

### Multi-Task Over-Engineering & RAM Exhaustion
Prior revisions spawned up to 10 independent FreeRTOS tasks to transmit sensor streams over BLE (`ble_gyro_raw_trans_task`, `ble_gyro_filt_trans_task`, `ble_accel_raw_trans_task`, `ble_accel_filt_trans_task`, `ble_mag_raw_trans_task`, `ble_mag_filt_trans_task`, `ble_baro_raw_trans_task`, `ble_baro_filt_trans_task`, `ble_att_quat_trans_task`, `ble_att_euler_trans_task`). Each task was allocated a 4096-byte stack, consuming $10 \times 4096 = 40.9\,\text{KB}$ of internal SRAM out of the ESP32-S3 memory pool.

Because the underlying physical BLE controller and radio hardware operate over a single serialization baseband, running 10 concurrent FreeRTOS tasks caused excessive scheduler thrashing, context-switching overhead, and stack memory waste without providing any physical concurrency.

### Unconditional Popping & BLE Stack Saturation
Tasks ran continuously and popped samples from middleware ring buffers even when no BLE client was connected or when the characteristic's Client Characteristic Configuration Descriptor (CCCD) notifications were disabled. This resulted in wasted CPU cycles and BLE NimBLE buffer overflow warnings.

### Duplicate Packet Structs & Missing SI Units
Legacy packet headers (`ble_packet.h`) declared duplicate 16-byte structs (`BleGyroPacket`, `BleAccelPacket`, `BleMagPacket`, `BleAttEulerPacket`) with identical memory layouts, lacking unified generic templating and explicit physical SI unit annotations.

---

## 3. Host Monitor Interaction Contract (`FCDatMon`)

The host monitor ([`FCDatMon`](file:///d:/OneDrive_MSFT/Work/Drone_Research/FCDatMon)) defines strict wire-compatibility constraints:

1. **Characteristic UUID Isolation**: Per `[Note 0009]`, each telemetry stream maps to an isolated BLE characteristic UUID. FCDatMon connects to **one characteristic UUID at a time** for high-frequency plotting.
2. **Dynamic Sample Stride**: The deserializer (`FCDatMon/core/ble_reader.py`) determines sample stride dynamically from received binary payload length $L$:
   $$S = 4\,(\text{TX timestamp}) + 4 \times N_{\text{series}}\,\text{bytes}$$
   - Vec3 (Gyro, Accel, Mag, Euler): $4 + 4 \times 3 = 16\,\text{bytes}$
   - Vec1 (Baro Altitude / Pressure): $4 + 4 \times 1 = 8\,\text{bytes}$
   - Quat (Attitude Quaternion): $4 + 4 \times 4 = 20\,\text{bytes}$
3. **Dual-Trigger Batching**: Per `[Note 0010]`, payloads flush when buffer reaches 31 samples ($31 \times 16 = 496\,\text{bytes}$, fitting safely under the 512-byte ATT MTU) OR when a $100\,\text{ms}$ timeout expires ($10\,\text{Hz}$ rate floor).

---

## 4. Core Architectural Decisions

### Rule 1: Single Unified Telemetry Dispatcher Task (`TASK_BLE_TELEM`)
The 10 legacy tasks are consolidated into a single low-priority telemetry dispatcher task:
- **Centralized Config (`include/config.h`)**:
  - `TASK_BLE_TELEM_NAME = "BLE_TELEM"`
  - `TASK_BLE_TELEM_ID` in `enum TaskIDs` (reducing total system tasks from 19 to 10)
  - `TASK_BLE_TELEM_PRIORITY = TASK_PRIORITY_LOW` (Priority 1)
  - `TASK_BLE_TELEM_STACK_SIZE = 4096` bytes
  - `TASK_BLE_TELEM_FREQ_HZ = 100` ($10\,\text{ms}$ evaluation loop)
  - `TASK_BLE_TELEM_CORE = tskNO_AFFINITY`
- **SRAM Savings**: Eliminating 9 duplicate task stacks reclaims **$36.8\,\text{KB}$ of internal SRAM**.

### Rule 2: Zero Dynamic Allocation & Static BSS Channel Instances
Each telemetry stream is encapsulated by a templated `BleTelemetryChannel<TPacket, MAX_SAMPLES>` class. Channel instances are declared `static` within Layer 4 (`src/core/telemetry/ble_telem_tasks.cpp`):
- All packet accumulation buffers reside in static BSS memory (~$3.9\,\text{KB}$ total across all channels).
- Zero memory is dynamically allocated (`new`/`malloc`) during flight loop execution.
- The FreeRTOS task stack of 4096 bytes handles only task execution framing with zero overflow risk.

### Rule 3: Subscription-Aware Gating (`is_ble_stream_subscribed`)
The BLE driver exposes `is_ble_stream_subscribed(uint16_t mask_bit)`:
```cpp
bool is_ble_stream_subscribed(uint16_t mask_bit){
  NimBLECharacteristic* p_char = get_char_by_mask(mask_bit);
  if(p_char != nullptr && p_char->getSubscribedCount() > 0) return true;
  return false;
}
```
If a characteristic has no active subscribers (`getSubscribedCount() == 0`), the dispatcher does **not** pop ring buffers or accumulate packets, completely eliminating unnecessary CPU usage and radio buffer thrashing.

### Rule 4: Generic Wire-Compatible Structs with Explicit Physical SI Units
Structs in `src/drivers/telemetry/ble_packet.h` use a generic `BleVec3Packet<float>` with strongly typed aliases:
- `BleGyroPacket` ($16\,\text{B}$, angular rate in $\text{rad/s}$, timestamp in $\mu\text{s}$)
- `BleAccelPacket` ($16\,\text{B}$, acceleration in $\text{m/s}^2$, timestamp in $\mu\text{s}$)
- `BleMagPacket` ($16\,\text{B}$, magnetic field in $\text{Gauss}$, timestamp in $\mu\text{s}$)
- `BleAttEulerPacket` ($16\,\text{B}$, Euler angles in $\text{deg}$, timestamp in $\mu\text{s}$)
- `BleBaroPacket` ($8\,\text{B}$, pressure in $\text{Pa}$ or altitude in $\text{m}$, timestamp in $\mu\text{s}$)
- `BleAttQuatPacket` ($20\,\text{B}$, unit quaternion, timestamp in $\mu\text{s}$)
Static assertions guarantee exact binary packet sizes matching FCDatMon's expected strides:
```cpp
static_assert(sizeof(BleGyroPacket) == 16, "BleGyroPacket size mismatch");
static_assert(sizeof(BleBaroPacket) == 8, "BleBaroPacket size mismatch");
static_assert(sizeof(BleAttQuatPacket) == 20, "BleAttQuatPacket size mismatch");
```

---

## 5. Architectural Verification & Backward Compatibility

| Subsystem Component | Legacy Architecture | Refactored Architecture | Verification / Compatibility |
|---|---|---|---|
| **FreeRTOS Tasks** | 10 separate tasks | 1 unified `ble_telem_task` | Scheduler overhead reduced; 36.8 KB SRAM saved |
| **GATT Wire Stride** | 16B / 8B / 20B Little-Endian | 16B / 8B / 20B Little-Endian | 100% backward wire-compatible with `FCDatMon` |
| **Flush Dynamics** | 496B capacity OR 100ms timeout | 496B capacity OR 100ms timeout | Identical dual-trigger cadence preserved |
| **Connection State** | Blind free-running pops | Subscription-gated via CCCD | Zero buffer contention when host is disconnected |
| **Physical Units** | Implicit / Undocumented | Explicit comments on all members | Fully compliant with team architecture rules |

---

## 6. Cross-References & Traceability
- [[0001-taskConfigMiddlewareIsrDecision]]: Centralized task configurations.
- [[0002-taskEnumsAndHalAbstractionDecision]]: Target-agnostic HAL and explicit typing.
- [[0009-distinctBleGattCharacteristicUuidIsolation]]: Dedicated GATT characteristic UUID allocation.
- [[0010-bleMinimumPacketFrequencyAndDualTriggerFlush]]: Telemetry dual-trigger flushing.
- [[0011-variableBleTelemetryParsingAndZeroHandshakeDesign]]: Binary telemetry encoding and stride layout.
- [[0016-coreDirectoryCompactionAndDomainConsolidation]]: Core compaction into unified domains.
- [[0022-unifiedPressureRingBufferSensorVariableConsolidationAndAttitudeTaskRemoval]]: Unified pressure buffer and clean ring buffer naming (`accel`, `gyro`, `mag`, `pressure`).
