---
id: "0018"
title: MicroSD Flight Logging Architecture, POSIX Metadata Sync, User Button Shutdown & Binary Log Specification
author: Drone Research Team / Developer
date: 2026-10-07
time: 20:45:00 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#fc_v02", "#spi", "#sdcard", "#logging", "#fatfs", "#isr"]
references:
  - "[[0000-firmwareStructureLayerDecision]]"
  - "[[0001-taskConfigMiddlewareIsrDecision]]"
  - "[[0002-taskEnumsAndHalAbstractionDecision]]"
  - "[[0013-sdCardSpiStorageAndFlightLoggingArchitecture]]"
  - "[[0016-coreDirectoryCompactionAndDomainConsolidation]]"
  - "[[0019-universalFlightDataAnalyzerAndBinaryTelemetryAlignmentSpecification]]"
---

# Zettel 0018: MicroSD Flight Logging Architecture, POSIX Metadata Sync, User Button Shutdown & Binary Log Specification

## 1. Metadata & Scope
- **ID**: `0018`
- **Author**: Drone Research Team / Developer
- **Date**: 2026-10-07
- **Time**: 20:45:00 +07:00
- **Status**: Approved & Implemented
- **References**: `[Note 0000]`, `[Note 0001]`, `[Note 0002]`, `[Note 0013]`, `[Note 0016]`, `[Note 0019]`

---

## 2. Context & Root Cause Analysis

During hardware validation of the 100 Hz binary snapshot logging subsystem (`[Note 0013]`), two critical filesystem and task lifecycle anomalies were analyzed and resolved:

### A. FatFS 0-Byte Log File Phenomenon & POSIX Synchronization (`fsync`)
- **Symptom**: When inspecting the microSD card on a host workstation after logging flight data, generated log files (e.g. `LOG_0001.BIN`) appeared with **0 bytes** (empty), despite 512-byte sector writes succeeding (`write_sector` returning `true`).
- **Root Cause**: In ChaN's FatFS under ESP-IDF / Arduino VFS, sector data clusters are written to the flash medium via `fwrite()`, but the 32-byte **Directory Entry** containing the file size (`file_size`) is held in RAM cache to prevent premature flash wear on the directory sector. Standard C `fflush(_file)` flushes only libc user-space buffers down to VFS, but never invokes `fsync(fileno(_file))` (`f_sync`). When the card was removed or power disconnected, directory tables retained their initial 0-byte size.
- **Architectural Resolution**: Include `<unistd.h>` and invoke `fsync(fileno(_file))` inside `SDCardDriver::flush()` and `SDCardDriver::close()`. In addition, `sd_logger_task` runs an independent 1.0 Hz periodic timer (`LOG_METADATA_SYNC_PERIOD_MS = 1000`) calling `sd_driver->flush()` under `spi_bus_mutex`. This guarantees that at most 1.0 second of data or metadata can ever be lost in a mid-air power loss or collision.

### B. Mount Failures & Boot Memory Protection (Zero Stack Waste on Absent Card)
- **Symptom**: If the board booted without an SD card inserted, or if SPI initialization failed (`f_mount failed: (3) FR_NOT_READY`), dereferencing uninitialized file handles resulted in system crashes (`LoadProhibited`).
- **Root Cause**: Unconditional task creation prior to validating driver initialization and open file status.
- **Architectural Resolution**: 
  1. In `src/app/app.cpp`, wrap `xTaskCreatePinnedToCore(sd_logger_task, ...)` with `if(sd_driver.is_open())`. If the card is absent or mount fails, `sd_logger_task` is never allocated, saving 8,192 bytes (8 KB) of FreeRTOS task stack SRAM.
  2. In `src/core/blackbox/sd_logger_task.cpp`, provide defensive task guards:
     ```cpp
     if(!sd_driver->is_open()){
       vTaskSuspend(nullptr);
       return;
     }
     ```

### C. Circular Overwrite Pointer & Odometer Reset
- **Symptom**: File pointer sought repeatedly to the beginning on every 10 ms iteration once `LOG_MAX_FILE_SIZE_BYTES` was reached.
- **Architectural Resolution**: Inside `SDCardDriver::reset_file_pointer_to_data_start()`, reset `_bytes_written = sizeof(LogFileHeader);`, preventing circular seek thrashing.

### D. Hardware USER Button 3-Second Hold & Graceful Shutdown
- **Pin**: `USER_BTN_PIN = 38` (GPIO 38, configured with `INPUT_PULLUP`).
- **ISR Architecture**: Edge-triggered interrupt (`CHANGE`) on GPIO 38.
  - On `FALLING` edge (press): starts a 3000 ms FreeRTOS one-shot timer (`BtnHoldTmr`).
  - On `RISING` edge (release): cancels the timer (filtering short presses/accidental bumps).
  - On Timer Expiry: sets `sd_stop_logging_requested = true`, wakes `sd_logger_task` via `xTaskNotifyGive()`, and calls `trigger_sd_stop_led_feedback()`.
- **Visual Feedback**: WS2812 RGB LED (GPIO 2) pulses **3 times in Cyan (`RGB: 0, 255, 255`)** at 100 ms intervals under `status_led_mutex` to visually confirm that the file is safely closed and power can be disconnected.

---

## 3. MicroSD Binary Log Format Specification (`LOG_XXXX.BIN`)

This section documents the exact binary structure of flight log files for host parsers and offline analysis tools.

### A. 512-Byte Self-Describing File Header (`LogFileHeader`)
Every log file begins with a 512-byte header block (Sector 0):

```
+--------------------------------------------------------------------------------+
| Offset (B) | Field Name               | Type      | Size (B) | Description     |
+------------+--------------------------+-----------+----------+-----------------+
| 0x000      | magic                    | char[4]   | 4        | "FCLG" (ASCII)  |
| 0x004      | session_id               | uint16_t  | 2        | 1, 2, 3...      |
| 0x006      | board_name               | char[24]  | 24       | String (padded) |
| 0x01E      | boot_timestamp_us        | uint32_t  | 4        | Boot timestamp  |
| 0x022      | registered_packets_mask  | uint16_t  | 2        | Active bitmask  |
| 0x024      | limit_policy             | uint8_t   | 1        | 0=Stop, 1=Circ  |
| 0x025      | max_file_size_bytes      | uint32_t  | 4        | Max file limit  |
| 0x029      | circular_head_sector     | uint32_t  | 4        | Start sector (1)|
| 0x02D      | reserved                 | char[467] | 467      | Zero padding    |
+--------------------------------------------------------------------------------+
| Total Size = 512 Bytes (Exact 1 Sector)                                        |
+--------------------------------------------------------------------------------+
```

### B. Sensor Packet Payloads (Little-Endian, `<`)
Data packets follow immediately starting at byte offset `0x200` (512):

#### 1. `PKT_TYPE_IMU_RAW` (`0x01`) — 29 Bytes Total (32B with natural 4B alignment, see `[Note 0019]`)
- `[0x00]` `type`: `uint8_t` = `0x01`
- `[0x01]` `timestamp_us`: `uint32_t` (Microseconds since boot)
- `[0x05]` `gyro`: 3 $\times$ `float32` (`x, y, z`) [rad/s]
- `[0x11]` `accel`: 3 $\times$ `float32` (`x, y, z`) [$\text{m/s}^2$]
- Struct format string: `<B I 3f 3f`

#### 2. `PKT_TYPE_IMU_FILT` (`0x02`) — 29 Bytes Total (32B with natural 4B alignment)
- `[0x00]` `type`: `uint8_t` = `0x02`
- `[0x01]` `timestamp_us`: `uint32_t`
- `[0x05]` `gyro_filt`: 3 $\times$ `float32` (`x, y, z`) [rad/s]
- `[0x11]` `accel_filt`: 3 $\times$ `float32` (`x, y, z`) [$\text{m/s}^2$]
- Struct format string: `<B I 3f 3f`

#### 3. `PKT_TYPE_MAG_RAW` (`0x03`) — 17 Bytes Total (20B with natural 4B alignment)
- `[0x00]` `type`: `uint8_t` = `0x03`
- `[0x01]` `timestamp_us`: `uint32_t`
- `[0x05]` `mag`: 3 $\times$ `float32` (`x, y, z`) [$\mu\text{T}$]
- Struct format string: `<B I 3f`

#### 4. `PKT_TYPE_MAG_FILT` (`0x04`) — 17 Bytes Total (20B with natural 4B alignment)
- `[0x00]` `type`: `uint8_t` = `0x04`
- `[0x01]` `timestamp_us`: `uint32_t`
- `[0x05]` `mag_filt`: 3 $\times$ `float32` (`x, y, z`) [$\mu\text{T}$]
- Struct format string: `<B I 3f`

#### 5. `PKT_TYPE_BARO_RAW` (`0x05`) — 9 Bytes Total (12B with natural 4B alignment)
- `[0x00]` `type`: `uint8_t` = `0x05`
- `[0x01]` `timestamp_us`: `uint32_t`
- `[0x05]` `pressure`: `float32` [Pa]
- Struct format string: `<B I f`

#### 6. `PKT_TYPE_BARO_FILT` (`0x06`) — 9 Bytes Total (12B with natural 4B alignment)
- `[0x00]` `type`: `uint8_t` = `0x06`
- `[0x01]` `timestamp_us`: `uint32_t`
- `[0x05]` `pressure_filt`: `float32` [Pa]
- Struct format string: `<B I f`

#### 7. `PKT_TYPE_ATT_QUAT` (`0x07`) — 21 Bytes Total (24B with natural 4B alignment)
- `[0x00]` `type`: `uint8_t` = `0x07`
- `[0x01]` `timestamp_us`: `uint32_t`
- `[0x05]` `q[4]`: 4 $\times$ `float32` ($q_0, q_1, q_2, q_3$)
- Struct format string: `<B I 4f`

#### 8. `PKT_TYPE_ATT_EULER` (`0x08`) — 17 Bytes Total (20B with natural 4B alignment)
- `[0x00]` `type`: `uint8_t` = `0x08`
- `[0x01]` `timestamp_us`: `uint32_t`
- `[0x05]` `euler`: 3 $\times$ `float32` (roll, pitch, yaw) [deg]
- Struct format string: `<B I 3f`

### C. Sector Framing & Padding Rules
1. Sectors are strictly aligned to 512 bytes (`uint8_t sector_buf[512]`).
2. When a sector is flushed (due to buffer full or 100 ms timeout), remaining unused bytes at the end of the 512-byte block are zeroed (`0x00`).
3. Parsers encountering `type == 0x00` advance byte-by-byte until a valid packet type identifier is located.

---

## 4. Traceability & Affected Files

- `include/config.h`:
  - `ACTIVE_SD_LOG_PACKETS_MASK`: SSoT mask controlling entire SD logging subsystem
  - `LOG_METADATA_SYNC_PERIOD_MS = 1000`
  - `USER_BTN_HOLD_DURATION_MS = 3000`
  - `LED_COLOR_SD_STOP_R = 0`, `LED_COLOR_SD_STOP_G = 255`, `LED_COLOR_SD_STOP_B = 255` (Cyan)
- `src/drivers/storage/sd_card_driver.cpp`:
  - Added `<unistd.h>` and `fsync(fileno(_file))` to `flush()` and `close()`.
  - Reset `_bytes_written = sizeof(LogFileHeader);` in `reset_file_pointer_to_data_start()`.
- `src/middleware/ipc.h` & `src/middleware/ipc.cpp`:
  - Added `extern volatile bool sd_stop_logging_requested;`.
  - Implemented `trigger_sd_stop_led_feedback()` (triple Cyan pulse under `status_led_mutex`).
- `src/core/interface/user_button.h` & `src/core/interface/user_button.cpp`:
  - Attached non-blocking edge ISR on `USER_BTN_PIN` (GPIO 38).
  - FreeRTOS 3000 ms timer triggering clean shutdown.
- `src/core/blackbox/sd_logger_task.cpp`:
  - Handles `sd_stop_logging_requested` shutdown signal.
  - Implemented 1.0 Hz periodic `fsync` directory sync timer.
- `src/app/app.cpp`:
  - Calls `user_button_init();`.
  - Guards task creation with `if(sd_driver.is_open())`.
- `[[0019-universalFlightDataAnalyzerAndBinaryTelemetryAlignmentSpecification]]`:
  - Documents compiler 4-byte struct padding discovery and companion universal flight data analyzer tool.
