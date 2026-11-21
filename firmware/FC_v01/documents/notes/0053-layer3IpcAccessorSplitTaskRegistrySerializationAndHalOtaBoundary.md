---
id: "0053"
title: Layer 3 IPC Accessor Split, Task-Registry Serialization & HAL_OTA Boundary
author: Drone Research Team / OpenCode
date: 2026-11-21
time: 17:56:00 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#ipc", "#middleware", "#hal", "#ota", "#layering"]
references:
  - "[[0000-firmwareStructureLayerDecision]]"
  - "[[0002-taskEnumsAndHalAbstractionDecision]]"
  - "[[0038-osalLayer3IpcDecouplingAndInterfaceTasksConsolidation]]"
  - "[[0042-layer5ExclusiveTaskSuspensionAuthorityAndOtaDecoupling]]"
  - "[[0046-fsmDirectEventReportingAndControlledLayerInversionException]]"
  - "[[0048-halSpiInterfaceSegregationAndSdCardDriverDecoupling]]"
  - "[[0050-pureOnDemandBleOtaMigrationAndBootWindowRetirement]]"
---

# Zettel 0053: Layer 3 IPC Accessor Split, Task-Registry Serialization & HAL_OTA Boundary

## 1. Metadata & Scope
- **ID**: `0053`
- **Author**: Drone Research Team / OpenCode
- **Date**: 2026-11-21
- **Time**: 17:56:00 +07:00
- **Status**: **Implemented, not yet compiled.** Build gate owed (see §5).
- **Amends**: `[[0038-osalLayer3IpcDecouplingAndInterfaceTasksConsolidation]]` (its Decision 2 is now genuinely executed), `[[0002-taskEnumsAndHalAbstractionDecision]]` Rule 6 (ownership/types/access), `[[0050-pureOnDemandBleOtaMigrationAndBootWindowRetirement]]` (`spawn_ble_ota_task(HAL_OTA&)` signature).
- **Scope**: Layer 3 (`src/middleware/`), Layer 4 (`src/core/telemetry/`), Layer 1/2 (`src/platforms/`), `include/ipc_types.h`.

---

## 2. Context & Motivation

`0002` Rule 6 demanded that *everything* shared go through one `ipc.h`. That centralized the **storage** correctly but not the **access**: the header grew to 30 `extern` symbols against 3 API functions, 17 translation units parsed NimBLE, FreeRTOS and the math engine to obtain a handle, and shared state was mutated directly from four layers. Three real defects were found by that contract (a Layer 4 queue allocation, a logger suspending itself, a Layer 4 write to the task registry) — which is the proof the policy works and the reason it was kept rather than abandoned.

Separately, OTA was the last hardware interaction bypassing Layer 1: 11 `esp_ota_*` / `esp_partition_*` calls sat directly in a Layer 4 task file.

---

## 3. Architectural Decision

1. **Four headers, one owner.** The hub is split by responsibility while `src/middleware/ipc.cpp` stays the single instantiation site:
   - `include/ipc_types.h` — POD contracts that cross an IPC boundary (`RcChannels`, `LedCommand`, `CalibReportMsg`, `AttCmd`). No OS type, no vendor type, no template math. It lives in `include/` — not `middleware/` — because Layer 2's `fs_ia6b.h` fills `RcChannels`; putting the contract under `middleware/` would recreate the upward include the split exists to remove.
   - `src/middleware/ipc_handles.h` — the `extern` handles, queues and ring buffers.
   - `src/middleware/ipc_lifecycle.h` — `init_ipc()`, on-demand create/destroy, and the task-registry accessors.
   - `src/middleware/ipc.h` — umbrella that includes the three; migration may narrow includes file by file.
2. **Registry mutations are serialized.** `ipc_register_task(task_id, handle)` / `ipc_get_task(task_id)` in Layer 3 are the only mutation path; both run inside `SYS_ENTER_CRITICAL()`. Layer 5's task-creation site in `app.cpp` may still populate a slot at creation. This removes the two-writer race where a Layer 4 OTA teardown nulled its own slot while `app_suspend_all_tasks()` was iterating the array.
3. **Ring buffers are deliberately NOT wrapped.** They are SPSC and lock-free on the 500 Hz / 2 ms path; `RingBuffer::push()/front()/back()/count()` already take a short critical section internally. Adding an `ipc_push_accel()` with validation or a mutex would inject jitter into the flight loop — a reliability regression, not an improvement. Direct access is sanctioned **provided** the SPSC discipline is stated at the declaration (`ipc_handles.h`).
4. **Hardware access is a HAL port.** `HAL_OTA` (`src/platforms/hal/hal_ota.h`) exposes `begin / write / end_and_set_boot / abort / running_partition_size / restart`; the ESP implementation lives in `src/platforms/targets/espressif/ota.cpp`; `get_ota_hal()` mirrors `get_i2c0_bus()`. Layer 4 receives the dependency by injection — `spawn_ble_ota_task(HAL_OTA&)` — instead of calling an accessor. OTA is abstractable precisely because it is six functions with primitive parameters; BLE and SD are not, because each owns a protocol state machine (see `[[0048-halSpiInterfaceSegregationAndSdCardDriverDecoupling]]` for that distinction).
5. **Event reporting is a Layer 3 contract.** `src/middleware/flight_events.h` carries the `EVT_*` masks and the two declarations (`fsm_set_event`, `fsm_clear_event`). Layer 4 reports; Layer 5 decides. This bounds the controlled inversion granted by `[[0046-fsmDirectEventReportingAndControlledLayerInversionException]]` to one header, so Layer 4 never compiles the FSM transition table.

---

## 4. Impacted Files

| Layer | File | Action |
|---|---|---|
| Contracts | `include/ipc_types.h` | **New.** POD contracts moved out of `middleware/` |
| Layer 3 | `src/middleware/ipc.h` | Reduced to umbrella; no vendor include, no NimBLE class, no `ble_ota_packet.h` |
| Layer 3 | `src/middleware/ipc_handles.h` | **New.** 30 `extern` handles + SPSC discipline block |
| Layer 3 | `src/middleware/ipc_lifecycle.h` | **New.** `init_ipc`, on-demand OTA queue, registry accessors |
| Layer 3 | `src/middleware/flight_events.h` | **New.** `EVT_*` masks + `fsm_set_event/clear_event` declarations |
| Layer 3 | `src/middleware/ipc.cpp` | Owner of every object; `on_ble_ota_event()` sink; registry accessors |
| Layer 1/2 | `src/platforms/hal/hal_ota.h`, `src/platforms/targets/espressif/ota.cpp` | **New.** `HAL_OTA` interface + ESP implementation |
| Layer 4 | `src/core/telemetry/ble_ota_tasks.cpp` | 0 direct `app_task_handles[]` writes; 0 `esp_ota_*` calls; takes `HAL_OTA&` |
| Layer 4 | `src/core/telemetry/rc_receiver_task.cpp` | Includes `middleware/flight_events.h`, not `app/fsm.h` |
| Rules | `.agents/AGENTS.md` §2, `0002` Rule 6 | Ownership / Types / Access reworded; no blanket "everything through `ipc.h`" |

---

## 5. Verification

- `rg "app_task_handles" src/core/telemetry/ble_ota_tasks.cpp` → **0** direct writes.
- `rg "esp_ota_" src/core/telemetry/ble_ota_tasks.cpp` → **0** live calls (one comment remains).
- `rg 'include "app/fsm.h"' src/core` → **0** files.
- **Not verified by compilation.** No build has run since the 0052 refactor; `pio run -e fc-v01` and `-e fc-v01-ota` plus the OTA smoke test are still owed before this note can be marked *verified*. Per `.agents/AGENTS.md`, compilation is a developer action.

---

## 6. Cross-References & Traceability
- [[0000-firmwareStructureLayerDecision]]: layer tree and the downward-only dependency rule this split preserves.
- [[0002-taskEnumsAndHalAbstractionDecision]]: Rule 6 is the policy this note re-expresses as ownership/types/access; Rule 3 is the HAL-interaction rule that forced `HAL_OTA`.
- [[0038-osalLayer3IpcDecouplingAndInterfaceTasksConsolidation]]: Decision 2 declared "Executed" prematurely; this note is the completion of its payload-decoupling and registry halves.
- [[0042-layer5ExclusiveTaskSuspensionAuthorityAndOtaDecoupling]]: the suspension authority is the consumer that made direct registry writes a race.
- [[0046-fsmDirectEventReportingAndControlledLayerInversionException]]: the inversion this note bounds to `flight_events.h`.
- [[0048-halSpiInterfaceSegregationAndSdCardDriverDecoupling]]: same HAL-boundary test, different verdict — SD keeps its protocol, OTA does not need one.
- [[0050-pureOnDemandBleOtaMigrationAndBootWindowRetirement]]: on-demand OTA allocation, now paired with an injected `HAL_OTA&`.
- [[0054-baselineCommitWorktreeRetirementAndOnedriveGitHazard]]: the commit/verification context in which this landed.
