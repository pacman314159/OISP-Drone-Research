---
id: "0050"
title: Pure On-Demand BLE OTA Architecture & Legacy Boot Window Retirement
author: Drone Research Team / Developer
date: 2026-10-29
time: 16:45:00 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#ble", "#ota", "#fsm", "#memory"]
references:
  - "[[0000-firmwareStructureLayerDecision]]"
  - "[[0001-taskConfigMiddlewareIsrDecision]]"
  - "[[0015-bleOtaArchitectureHardeningAndResourceCleanup]]"
  - "[[0039-onDemandBleOtaInDevelopmentStateAndDependencyInversionArchitecture]]"
  - "[[0042-layer5ExclusiveTaskSuspensionAuthorityAndOtaDecoupling]]"
---

# Zettel 0050: Pure On-Demand BLE OTA Architecture & Legacy Boot Window Retirement

## 1. Metadata & Scope
- **ID**: `0050`
- **Author**: Drone Research Team / Developer
- **Date**: 2026-10-29
- **Time**: 16:45:00 +07:00
- **Status**: Approved & Implemented
- **Module Context**: Layer 5 (`src/app/app.cpp`), Layer 4 (`src/core/telemetry/ble_ota_tasks.h/.cpp`), Layer 3 (`src/middleware/ipc.h/.cpp`), Layer 2 (`src/drivers/telemetry/ble_driver.cpp`), and Host Tooling (`tools/ble_ota_uploader.py`).

---

## 2. Problem Statement & Legacy Entanglements
Under `[Note 0015]`, the firmware implemented a 10-second boot-time listening window (`OTA_BOOT_WINDOW_TIMEOUT_MS = 10000`). When on-demand OTA was later introduced under `[Note 0039]` and `[Note 0042]`, the legacy boot task creation was inadvertently left active in `app_start()`.

This introduced several architectural regressions and operational defects:
1. **Spurious Boot-Time Timeout Log**:
   Every cold boot logged `[BLE_OTA] 10000 ms boot window expired without OTA trigger. Terminating OTA task.`, creating confusion as to whether the on-demand mechanism was functioning.
2. **Boot Heap Allocation Waste**:
   `init_ipc()` in Layer 3 unconditionally allocated `ota_data_queue` (~16.5 KB SRAM) at startup, occupying precious internal SRAM during initial sensor calibrations and EKF convergence before being deleted 10 seconds later.
3. **Dual-Personality Code Branching**:
   `ble_ota_task` maintained dual execution logic via an `arg != nullptr` pointer check, interleaving the obsolete `ulTaskNotifyTake` timeout block with active OTA flash programming logic.

---

## 3. Core Architectural Decisions

### Rule 1: Complete Retirement of Boot-Time OTA Task Spawning
`xTaskCreatePinnedToCore(ble_ota_task, ...)` is completely removed from `app_start()` in Layer 5 (`src/app/app.cpp`). Cold boot time and task scheduling are 100% dedicated to operational sensor DAQ, estimators, and flight FSM initialization.

### Rule 2: Lazy Dynamic Allocation of OTA Resources
`ota_data_queue` is no longer allocated in `init_ipc()`:
- `init_ipc()` returns without claiming OTA queue heap memory.
- `ota_data_queue` (~16.5 KB SRAM) and `ble_ota_task` (4 KB stack) are allocated strictly on-demand within `spawn_ble_ota_task()` when Layer 5 authorizes `OTA_CMD_START`.
- Memory is released immediately via MCU reboot upon flash completion or abort.

### Rule 3: Direct Task Execution Without Boot-Wait Branching
`ble_ota_task` in Layer 4 (`src/core/telemetry/ble_ota_tasks.cpp`) is simplified into a single-purpose linear state engine:
- The parameter `immediate_start` is eliminated from `spawn_ble_ota_task()`.
- The 10-second `ulTaskNotifyTake` wait block and `OTA_BOOT_WINDOW_TIMEOUT_MS` constant are retired.
- When `ble_ota_task` is spawned, it immediately initializes the target OTA partition and starts receiving binary chunks.

### Rule 4: Clean Layer 2 Signaling
In `src/drivers/telemetry/ble_driver.cpp`, `OtaControlCallbacks::onWrite` delegates authorization strictly to Layer 5 (`s_ota_trigger_cb()`). Redundant `xTaskNotifyGive()` token calls on `TASK_BLE_OTA_ID` are eliminated.

### Rule 5: Allocation Ownership Retained by Layer 3
**Allocation ownership (amended 2026-11-18).** The on-demand policy is unchanged, but the allocation call itself belongs to Layer 3. `src/core/telemetry/ble_ota_tasks.cpp` calls `ipc_create_ota_queue(sizeof(OtaDataChunk))` from `spawn_ble_ota_task()`; `bool ipc_create_ota_queue(size_t chunk_size)` and `void ipc_destroy_ota_queue()` are declared in `src/middleware/ipc.h` and implemented in `src/middleware/ipc.cpp`. Layer 3 performs the allocation; the allocation call itself requires no packet header because the payload size is supplied by the caller, so the payload type stays owned by Layer 4. Note that `src/middleware/ipc.h` still imports `drivers/telemetry/ble_ota_packet.h` for the NimBLE disconnect callback in `ipc.cpp` — removing that remaining Layer 3 → Layer 2 dependency is tracked separately and is *not* part of this amendment. This satisfies `0002` §73–74 without reintroducing the ~16.5 KB boot-time SRAM cost.

**Amendments 2026-11-19 (audit items C1, C4, C7, C15).**
- *C1:* `src/middleware/ipc.h` no longer imports `ble_ota_packet.h`; the include lives in `src/middleware/ipc.cpp:7`. The stale sentence above is kept as history.
- *C4:* task-registry mutations in `spawn_ble_ota_task()` and in the OTA task's exit paths now go through `ipc_register_task()` / `ipc_get_task()`. This note's on-demand spawn decision is unchanged; only the registry access moved behind the Layer 3 API.
- *C7:* the signature is now `bool spawn_ble_ota_task(HAL_OTA& ota)`. Layer 5 injects `get_ota_hal()`; the `esp_ota_*` / `esp_partition_*` calls moved verbatim to `src/platforms/targets/espressif/ota.cpp`.
- *C15:* the declaration home named above is now split across `include/ipc_types.h`, `middleware/ipc_handles.h` and `middleware/ipc_lifecycle.h`; `middleware/ipc.h` remains the umbrella and `ipc.cpp` the single owner.

---

## 4. Resource & Memory Accounting

| Parameter | Legacy State (Note 0015 / 0039) | Pure On-Demand (Note 0050) | Net Saving at Boot |
| :--- | :---: | :---: | :---: |
| **Boot Task Handles** | 1 (`ble_ota_task`) | 0 | -1 Task Handle |
| **Boot Task Stack** | 4,096 bytes (4 KB) | 0 bytes | **+4,096 bytes SRAM** |
| **Boot Queue Heap** | 16,448 bytes (~16.5 KB) | 0 bytes | **+16,448 bytes SRAM** |
| **Total Cold-Boot RAM Reclaimed** | — | — | **+20,544 bytes (~20.5 KB)** |
| **Boot Log Noise** | 10s timeout log | 0 log messages | Clean UART output |

---

## 5. Cross-References & Traceability
- **Supersedes**: Rule 1 (10-Second Boot-Up Security Window) of `[[0015-bleOtaArchitectureHardeningAndResourceCleanup]]`.
- **Extends & Cleans**: `[[0039-onDemandBleOtaInDevelopmentStateAndDependencyInversionArchitecture]]` by eliminating dual fallback paths.
- **Enforces**: Layer 5 exclusive task suspension authority defined in `[[0042-layer5ExclusiveTaskSuspensionAuthorityAndOtaDecoupling]]`.
- **Complies With**: 5-layer hierarchy in `[[0000-firmwareStructureLayerDecision]]` and centralized task configuration in `[[0001-taskConfigMiddlewareIsrDecision]]`.
