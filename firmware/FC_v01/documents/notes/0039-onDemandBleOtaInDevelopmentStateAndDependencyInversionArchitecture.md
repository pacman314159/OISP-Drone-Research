---
id: "0039"
title: On-Demand BLE OTA Firmware Flashing in DEVELOPMENT State via Dependency Inversion
author: Drone Research Team / Developer
date: 2026-10-27
time: 22:15:00 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#ble", "#ota", "#fsm", "#dependency_inversion"]
references:
  - "[[0000-firmwareStructureLayerDecision]]"
  - "[[0001-taskConfigMiddlewareIsrDecision]]"
  - "[[0006-taskCreationConfigEnumsAndTaskArgsPassingDecision]]"
  - "[[0015-bleOtaArchitectureHardeningAndResourceCleanup]]"
  - "[[0016-coreDirectoryCompactionAndDomainConsolidation]]"
  - "[[0038-osalLayer3IpcDecouplingAndInterfaceTasksConsolidation]]"
  - "[[0042-layer5ExclusiveTaskSuspensionAuthorityAndOtaDecoupling]]"
---

# Zettel 0039: On-Demand BLE OTA Firmware Flashing in DEVELOPMENT State via Dependency Inversion

## 1. Metadata & Scope
- **ID**: `0039`
- **Author**: Drone Research Team / Developer
- **Date**: 2026-10-27
- **Time**: 22:15:00 +07:00
- **Status**: Approved & Implemented
- **References**: `[Note 0000]`, `[Note 0001]`, `[Note 0006]`, `[Note 0015]`, `[Note 0016]`, `[Note 0038]`

---

## 2. Problem Statement & Context
Under `[Note 0015]`, `ble_ota_task` was constrained to a strictly enforced 10-second listening window (`OTA_BOOT_WINDOW_TIMEOUT_MS = 10000`) after boot, after which it deleted `ota_data_queue` (~16.5 KB SRAM) and self-deleted (`vTaskDelete`) to preserve real-time RAM for the 500 Hz flight loop.

While sound for in-flight protection, this created severe development friction:
1. **Compilation Race Condition**: Triggering `pio run -e fc-v01-ota` builds the binary first; if the board is reset beforehand, compilation routinely exceeds 10 seconds, causing the OTA uploader to connect to an expired listener.
2. **Repetitive Physical Resets**: Developers had to manually press the hardware `RST` button on the flight controller for every upload cycle.
3. **In-Session Upload Requirement**: Remote reboot over BLE drops the Bluetooth connection, triggering OS Bluetooth stack re-scan and reconnection latency. The target workflow requires uploading directly within the active BLE connection.

---

## 3. Core Architectural Decisions

### Rule 1: Dependency Inversion for Layer 2 BLE Driver Decoupling
To allow Layer 2 (`src/drivers/telemetry/ble_driver.cpp`) to trigger on-demand OTA authorization without importing or coupling to Layer 5 (`src/app/fsm.h`), Layer 2 defines a pure function pointer callback:

```cpp
typedef bool (*BleOtaTriggerCallback)();
void set_ble_ota_trigger_callback(BleOtaTriggerCallback cb);
```

During `app_start()`, Layer 5 (`src/app/app.cpp`) injects its concrete authorization function:
```cpp
static bool on_ble_ota_requested(){
  if(fsm_get_state() != DEVELOPMENT) return false;
  return spawn_ble_ota_task(true);
}
```
Calls flow downward at compile time; runtime events trigger upward through function pointers with zero `#include` layer violations.

### Rule 2: On-Demand Task & Queue Spawning (`spawn_ble_ota_task`)
In Layer 4 (`src/core/telemetry/ble_ota_tasks.cpp`):
1. When `CMD_OTA_START` arrives outside the 10-second boot window and is authorized, `spawn_ble_ota_task(immediate_start=true)` allocates `ota_data_queue` on demand:
   ```cpp
   if(ota_data_queue == nullptr)
     ota_data_queue = xQueueCreate(OTA_DATA_QUEUE_DEPTH, sizeof(OtaDataChunk));
   ```
2. The task is created with a non-null argument (`(void*)1`), signaling `ble_ota_task` to bypass the 10-second wait and immediately enter OTA update mode.
3. `app_suspend_all_tasks()` is called instantly, halting all flight control, sensor DAQ, and FSM loops so 100% CPU and flash bus bandwidth are allocated to the incoming firmware stream.

### Rule 3: Strict In-Flight Safety Lockout
On-demand OTA outside the 10-second boot window is **strictly rejected** (`OTA_RESP_ERR_ABORT`) whenever the system is not in `DEVELOPMENT` state. Active flight states (`READY`, `TAKEOFF`, `FLIGHT`, `LEVELOUT`, `CRASHED`) are completely protected against OTA takeover.

### Rule 4: Protocol Extension for Remote Reboot (`OTA_CMD_REBOOT`)
Opcode `OTA_CMD_REBOOT = 0x04` is added to `ble_ota_packet.h` and handled in `ble_driver.cpp`, enabling client tools to dispatch a soft MCU reset over BLE on demand via `--reboot`.

### Rule 5: Link Layer Supervision & Anti-Starvation Hardening
To prevent unexpected disconnects (`WinError -2147023673` / `0x800704C7`) during continuous chunk transmission:
1. **Extended Supervision Timeout**: Increased BLE connection supervision timeout in `pServer->updateConnParams` from 2.0s (`timeout=200`) to 5.0s (`timeout=500`) in `src/middleware/ipc.cpp` to prevent link drops during SPI flash erase latency spikes.
2. **Flash Write Backpressure Margin**: Increased queue dispatch timeout in `OtaDataCallbacks::onWrite` from 100 ms to 500 ms in `src/drivers/telemetry/ble_driver.cpp` to accommodate sector erase times under bus contention.
3. **Anti-Starvation Scheduler Yield**: In `ble_ota_task`, inserted a 1 ms yield (`vTaskDelay(pdMS_TO_TICKS(1))`) every 4 KB sector boundary (~8 chunks), guaranteeing the NimBLE host task and FreeRTOS IDLE task receive CPU slices to service link layer keepalives and feed system watchdogs.

---

## 4. Cross-References & Traceability
- Extends the BLE OTA lifecycle established in `[[0015-bleOtaArchitectureHardeningAndResourceCleanup]]`.
- Preserves the 5-layer downward dependency hierarchy defined in `[[0000-firmwareStructureLayerDecision]]` and `[[0001-taskConfigMiddlewareIsrDecision]]`.
- Complies with task creation and ID passing invariants from `[[0006-taskCreationConfigEnumsAndTaskArgsPassingDecision]]`.
- Adheres to Layer 3 OSAL & IPC decoupling rules from `[[0038-osalLayer3IpcDecouplingAndInterfaceTasksConsolidation]]`.
- Task suspension authority strictly reserved to Layer 5 per `[[0042-layer5ExclusiveTaskSuspensionAuthorityAndOtaDecoupling]]`, fully decoupling `ble_ota_tasks.cpp` from `app/app.h`.
