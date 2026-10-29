---
id: "0015"
title: BLE OTA Lifecycle Architecture, Resource Stewardship & Host Stack Decoupling
author: Drone Research Team / Developer
date: 2026-10-05
time: 20:25:00 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#ble", "#ota", "#freertos", "#memory"]
references:
  - "[[0000-firmwareStructureLayerDecision]]"
  - "[[0001-taskConfigMiddlewareIsrDecision]]"
  - "[[0002-taskEnumsAndHalAbstractionDecision]]"
  - "[[0006-taskCreationConfigEnumsAndTaskArgsPassingDecision]]"
  - "[[0009-distinctBleGattCharacteristicUuidIsolation]]"
  - "[[0012-ws2812HardwareIsolationAndMutexEncapsulationRule]]"
  - "[[0016-coreDirectoryCompactionAndDomainConsolidation]]"
---

# Zettel 0015: BLE OTA Lifecycle Architecture, Resource Stewardship & Host Stack Decoupling

## 1. Metadata & Context
- **ID**: `0015`
- **Author**: Drone Research Team / Developer
- **Date**: 2026-10-05
- **Time**: 20:25:00 +07:00
- **Status**: Approved & Enforced
- **References**: `[Note 0000]`, `[Note 0001]`, `[Note 0002]`, `[Note 0006]`, `[Note 0009]`, `[Note 0012]`

---

## 2. Core Architectural Decisions

### Rule 1: 10-Second Boot-Up Security Window & Dynamic Memory Reclamation
To maintain wireless firmware update convenience without sacrificing in-flight security or real-time flight memory:
1. **Time-Bounded Listening Window**: The `ble_ota_task` runs for strictly `10000 ms` (`OTA_BOOT_WINDOW_TIMEOUT_MS`) upon cold boot or reset.
2. **Deterministic Heap Deallocation**: If no `OTA_CMD_START` handshake notification is received within the 10-second window, `ble_ota_task` deletes `ota_data_queue` via `vQueueDelete()`, reclaiming **~16.5 KB of internal SRAM** permanently for the 500 Hz flight loop, and self-deletes via `vTaskDelete(NULL)`.
3. **Late Trigger Rejection**: Any `OTA_CMD_START` command arriving after the boot window is actively rejected with `OTA_RESP_ERR_ABORT` rather than silently dropped.

### Rule 2: Layer 4 Task Identity & Zettel 0006 Rule 3 Preservation
In strict adherence to `[Note 0006]`, `ble_ota_task` in Layer 4 (`src/core/telemetry/ble_ota_tasks.cpp`) MUST maintain its assigned system task identity:
```cpp
const uint32_t task_id = TASK_BLE_OTA_ID;
```
Removing `task_id` is forbidden, ensuring uniform SystemView, telemetry, and system identification compliance across all Layer 4 tasks.

### Rule 3: NimBLE Host Task Decoupling & Stack Protection
To prevent BLE Link Supervision Timeouts (`BLE_HS_ETIMEOUT`) and call stack exhaustion:
1. **Non-Blocking Queue Dispatch**: GATT write callbacks in `OtaDataCallbacks::onWrite` must never block the NimBLE host task for more than 50 ms (`pdMS_TO_TICKS(50)`).
2. **Off-Stack Buffer Allocation**: Chunks transferred from GATT characteristics are buffered in static/dedicated memory (`s_incoming_ota_chunk`) rather than on the NimBLE host thread call stack, safeguarding the stack from overflow.

### Rule 4: Client-Side ATT MTU Adaptation & Error Unblocking
In `tools/ble_ota_uploader.py`:
1. **Dynamic MTU Clamping**: The uploader inspects `client.mtu_size` after BLE connection and clamps chunk size dynamically to `min(chunk_size, client.mtu_size - 3)` to support arbitrary host Bluetooth adapters.
2. **Dual-Event Error Unblocking**: Any error response code (`RESP_ERR_*`) sets both `ready_event` and `finish_event` to prevent client deadlocks during initial handshakes.

### Rule 5: Environment Simplification & Local Project Compilation
The obsolete `esp32-s3-devkitc-1-n8` environment is retired across all tools and scripts. All PlatformIO builds target strictly:
- `fc-v01` (UART / JTAG debugging)
- `fc-v01-ota` (BLE OTA upload)
Build artifacts reside exclusively in `.pio/build/` inside the local repository, eliminating machine-specific absolute path dependencies.

### Rule 6: Bluetooth 5.0 2M PHY Negotiation & DLE 251 Link Layer Acceleration
To eliminate link layer throughput bottlenecks:
1. **Link Layer DLE**: Maximum ATT MTU of 512 bytes is accommodated by 251-byte Link Layer PDU fragmentation.
2. **2M PHY Support**: 2 Mbps physical radio modulation is preferred upon connection to halve packet on-air transmission time.

### Rule 7: Windowed WRITE_NR Pipelining with SPI Flash Backpressure Flow Control
To elevate BLE OTA throughput from ~14 KB/s to 60–90 KB/s without risking queue overflow or link timeouts:
1. **Windowed Pipelining**: In `tools/ble_ota_uploader.py`, chunks are streamed via `WRITE_NR` (`response=False`) in windows of 8 chunks (`DEFAULT_WINDOW_SIZE = 8`, ~3.9 KB), synchronizing with `response=True` on the 8th chunk boundary and the final EOF chunk. This provides ideal alignment with the 4 KB SPI flash sector boundaries while preventing Windows Bluetooth driver buffer saturation and unacknowledged packet loss.
2. **Deepened Buffer Queue**: `ota_data_queue` is configured to depth 32 (`OTA_DATA_QUEUE_DEPTH = 32`), buffering ~16.5 KB during active OTA to absorb SPI flash sector write latency spikes before being reclaimed permanently for flight control upon boot window expiry.
3. **Queue Overflow Guard**: `OtaDataCallbacks::onWrite` in `src/drivers/telemetry/ble_driver.cpp` detects queue full conditions and triggers an explicit abort notification rather than silent chunk loss.

---

## 3. Empirical Benchmarks & Hardware Observations (2026-10-06)
- **Baseline Speed (Unpipelined `response=True`)**: ~14 KB/s (~85s for 1.2 MB firmware image).
- **Peak Pipelined Speed (Windowed `WRITE_NR`, Window = 16)**: **~50 KB/s** (~24s for 1.2 MB firmware image).
- **Distance & RF Sensitivity**:
  - Max speed of ~50 KB/s is achieved when the flight controller board is placed in close proximity to the host computer / Bluetooth transceiver.
  - Throughput is sensitive to distance and physical orientation due to Link Layer packet retransmissions and RF path loss on the PCB antenna.
  - Operationally, ~50 KB/s provides an optimal trade-off of rapid flashing convenience without requiring complex compression overhead.

---

## 4. Cross-References & Traceability
- Extended by `[[0039-onDemandBleOtaInDevelopmentStateAndDependencyInversionArchitecture]]`: Adds on-demand OTA triggering in DEVELOPMENT mode without manual reset button cycles.
