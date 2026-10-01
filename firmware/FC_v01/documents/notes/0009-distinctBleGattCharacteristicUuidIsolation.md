---
id: "0009"
title: Distinct BLE GATT Characteristic UUID Isolation per Telemetry Task Decision
author: Drone Research Team / Developer
date: 2026-09-30
time: 16:58:00 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#ble", "#telemetry", "#gatt", "#uuid"]
references:
  - "[[0000-firmwareStructureLayerDecision]]"
  - "[[0001-taskConfigMiddlewareIsrDecision]]"
  - "[[0006-taskCreationConfigEnumsAndTaskArgsPassingDecision]]"
  - "[[0008-lpfAndCalibrationTaskArchitecture]]"
---

# Zettel 0009: Distinct BLE GATT Characteristic UUID Isolation per Telemetry Task Decision

## 1. Metadata & Context
- **ID**: `0009`
- **Date**: 2026-09-30
- **Status**: Approved & Enforced
- **References**: `[Note 0000]`, `[Note 0001]`, `[Note 0006]`, `[Note 0008]`

---

## 2. Core Architectural Decision

### Strict 1:1 Task-to-Characteristic Isolation
To eliminate packet collisions, multiplexing overhead, and subscription ambiguity on client software (FCDatMon):

1. **Centralized UUIDs (`include/config.h`)**:
   - `BLE_CHAR_VEC3_RAW_TELEM_UUID`: Dedicated characteristic for raw sensor streaming.
   - `BLE_CHAR_VEC3_FILT_TELEM_UUID`: Dedicated characteristic for filtered sensor streaming.

2. **Driver Isolation (`src/drivers/telemetry/`)**:
   - `send_vec3_raw_telemetry_notification()` $\rightarrow$ writes strictly to `p_vec3_raw_char`.
   - `send_vec3_filt_telemetry_notification()` $\rightarrow$ writes strictly to `p_vec3_filt_char`.

3. **Task Mapping (`src/core/telemetry/telem_tasks.cpp`)**:
   - `ble_vec3_raw_trans_task` transmits exclusively via the raw notification API.
   - `ble_vec3_filtered_trans_task` transmits exclusively via the filtered notification API.
