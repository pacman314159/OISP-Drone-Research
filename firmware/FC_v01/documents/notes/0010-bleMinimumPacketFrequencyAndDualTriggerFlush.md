---
id: "0010"
title: BLE Minimum Packet Frequency and Dual-Trigger Transmission Protocol
author: Drone Research Team / Developer
date: 2026-09-30
time: 19:23:00 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#ble", "#telemetry", "#timing"]
references:
  - "[[0001-taskConfigMiddlewareIsrDecision]]"
  - "[[0006-taskCreationConfigEnumsAndTaskArgsPassingDecision]]"
  - "[[0009-distinctBleGattCharacteristicUuidIsolation]]"
---

# Zettel 0010: BLE Minimum Packet Frequency and Dual-Trigger Transmission Protocol

## 1. Metadata & Context
- **ID**: `0010`
- **Date**: 2026-09-30
- **Status**: Approved & Enforced
- **References**: `[Note 0001]`, `[Note 0006]`, `[Note 0009]`

---

## 2. Core Architectural Decision

### Dual-Trigger Flushing Mechanism
To prevent low-frequency sensor telemetry (e.g., 75 Hz Magnetometer) from experiencing high transmission latency and "step-by-step chunky" plots on FCDatMon while retaining maximum MTU efficiency for 500 Hz IMU streams:

1. **Centralized Frequency Floor (`include/config.h`)**:
   `BLE_TELEM_MIN_PACKET_FREQ_HZ = 10` establishes a maximum transmission interval of $100\text{ ms}$ ($10\text{ Hz}$).

2. **Dual-Trigger Conditions (`src/core/telemetry/telem_tasks.cpp`)**:
   Both `ble_vec3_raw_trans_task` and `ble_vec3_filtered_trans_task` flush and transmit their payload buffer when **either** condition is satisfied:
   - **Capacity Trigger**: `ble_payload_count >= BLE_TELEM_SAMPLES_PER_PACKET` (31 samples). Governs 500 Hz Gyro/Accel, filling every ~60 ms for optimal MTU throughput.
   - **Timeout Trigger**: `ble_payload_count > 0 && (now - last_packet_send_tick) >= max_packet_interval_ticks` ($100\text{ ms}$). Governs 75 Hz Mag, dispatching partial batches (7–8 samples) every $100\text{ ms}$ to maintain a steady 10 Hz telemetry cadence.

3. **Clock Reset**:
   Every packet transmission (capacity- or timeout-triggered) resets `last_packet_send_tick = xTaskGetTickCount()` and clears `ble_payload_count = 0`.
