---
id: "0013"
title: Unified HAL_SPI Abstraction, 100 Hz Snapshot Logging & Multi-Rate MicroSD Storage
author: Drone Research Team / Developer
date: 2026-10-04
time: 18:33:00 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#spi", "#sdcard", "#logging", "#freertos"]
references:
  - "[[0000-firmwareStructureLayerDecision]]"
  - "[[0001-taskConfigMiddlewareIsrDecision]]"
  - "[[0002-taskEnumsAndHalAbstractionDecision]]"
  - "[[0006-taskCreationConfigEnumsAndTaskArgsPassingDecision]]"
  - "[[0007-systemViewTracingAndRTOSTimingAnalysisConventions]]"
  - "[[0010-bleMinimumPacketFrequencyAndDualTriggerFlush]]"
  - "[[0012-ws2812HardwareIsolationAndMutexEncapsulationRule]]"
  - "[[0016-coreDirectoryCompactionAndDomainConsolidation]]"
  - "[[0032-rcReceiverCommandBlackboxSdCardLoggingArchitecture]]"
---

# Zettel 0013: Unified HAL_SPI Abstraction, 100 Hz Snapshot Logging & Multi-Rate MicroSD Storage

## 1. Metadata & Context
- **ID**: `0013`
- **Author**: Drone Research Team / Developer
- **Date**: 2026-10-04
- **Time**: 18:33:00 +07:00
- **Status**: Approved & Enforced
- **References**: `[Note 0000]`, `[Note 0001]`, `[Note 0002]`, `[Note 0006]`, `[Note 0007]`, `[Note 0010]`, `[Note 0012]`

---

## 2. Core Architectural Decisions

### Rule 1: Unified `HAL_SPI` Abstraction & Shared Bus Decoupling (Layer 1)
- The physical SPI bus controller (`MOSI=11`, `SCK=12`, `MISO=13`) is abstracted through `HAL_SPI` in `src/platforms/hal/hal_spi.h`.
- The SPI bus is physically shared between the MicroSD card (`SPI_CS_SD_PIN = 8`), LoRa module (`SPI_CS_LORA_PIN = 39`), and peripheral devices (`SPI_CS_PERIP_PIN = 47`).
- Dedicated driver-level SPI peripherals are strictly prohibited. The hardware SPI host driver `ESP32_SPI` manages the shared hardware bus with GDMA hardware acceleration and FAT filesystem mounting.
- Global hardware bus accessor `get_spi_bus()` mirrors the existing `get_i2c0_bus()` interface pattern.

### Rule 2: Peripheral Driver Object Ownership (`SDCardDriver`, Layer 2)
- Aligned with `MPU6050` and `HMC5883L`, `SDCardDriver` in Layer 2 is target-agnostic and consumes `HAL_SPI&`, its designated Chip Select pin (`SPI_CS_SD_PIN`), and Card Detect pin (`SD_CARD_DET_PIN`).
- Global singleton driver instances or accessors for `SDCardDriver` in Layer 1 or Layer 2 are strictly forbidden.
- The driver is instantiated statically in Layer 5 (`src/app/app.cpp`) and passed directly to `sd_logger_task` in Layer 4 (`src/core/blackbox/sd_logger_task.cpp`) via `void* arg`.

### Rule 3: Autonomous 100 Hz Periodic Task & Zero-Overhead Ring Buffer Snapshot Draining (Layer 4)
- Rather than forcing high-frequency sensor DAQ loops to send task notifications, `sd_logger_task` runs autonomously at 100 Hz (`TASK_SD_LOGGER_FREQ_HZ = 100`) via `vTaskDelayUntil`.
  *(Note: FreeRTOS `xTaskNotify` from sensor tasks is intentionally avoided to keep 500 Hz flight loops zero-overhead and eliminate scheduler thrashing against SD write latency).*
- Must include an anti-windup overrun guard:
  ```cpp
  TickType_t now = xTaskGetTickCount();
  if((now - last_wake_time) > (period_ticks * 2))
    last_wake_time = now;
  vTaskDelayUntil(&last_wake_time, period_ticks);
  ```
- At each 10 ms cycle, the task drains accumulated multi-rate samples from thread-safe ring buffers via `get_snapshot()` and monotonic timestamp delta tracking:
  ```cpp
  if(!has_logged || static_cast<int32_t>(curr_ts - last_logged_ts) > 0)
  ```
- Dedicated symmetric Raw and Filtered packet tags exist for all primary sensors: IMU (`PKT_TYPE_IMU_RAW` / `PKT_TYPE_IMU_FILT`), Magnetometer (`PKT_TYPE_MAG_RAW` / `PKT_TYPE_MAG_FILT`), and Barometer (`PKT_TYPE_BARO_RAW` / `PKT_TYPE_BARO_FILT`).
- This design completely decouples the logging subsystem from critical 500 Hz flight loops, incurring strict zero CPU overhead in producer tasks.

### Rule 4: 512-Byte Sector Accumulator & Dual-Trigger Flush
- To maximize SD write performance and eliminate latency jitter, data packets are packed directly into a 512-byte RAM buffer.
- Buffer flushes occur under two conditions:
  1. **Full Trigger**: When appending a packet would exceed 512 bytes, the current 512-byte block is transferred to the SD card via GDMA under `spi_bus_mutex`, and the buffer index resets to 0.
  2. **Timeout Trigger**: If 100 ms elapses (`LOG_SECTOR_TIMEOUT_MS`) with pending unwritten bytes, the sector is padded to 512 bytes and flushed.

### Rule 5: Pure 4-Digit Incremental File Scheme & Self-Describing Binary Header
- Flight data files strictly adhere to 4-digit incremental filenames: `LOG_0001.BIN`, `LOG_0002.BIN`, matching the Zettelkasten note ID convention.
- At boot, `SDCardDriver::find_highest_log_id()` scans directory entries using standard POSIX `<dirent.h>` to discover the true maximum ID, safely skipping gaps or non-log files.
- Each file begins with a 512-byte `LogFileHeader` recording `session_id`, `board_name`, `boot_timestamp_us`, and `registered_packets_mask`.

---

## 3. Impact & Architectural Constraints
1. **HAL Purity**: Drivers across Layer 2 interact exclusively with `HAL_SPI&`.
2. **Real-Time Priority**: `sd_logger_task` runs at `TASK_PRIORITY_LOW` (Priority 1) with `tskNO_AFFINITY`, preventing SD card write latency spikes from starving the 500 Hz flight loop.
3. **IPC Mutex Protection**: All SPI bus transactions (both filesystem and raw transfers) are protected by `spi_bus_mutex` in `src/middleware/ipc.h`.
4. **Runtime Hardening & Binary Specification**: See `[[0018-sdCardLoggingRestorationAndUserButtonShutdown]]` for runtime metadata sync analysis, button shutdown, and binary log specification.
