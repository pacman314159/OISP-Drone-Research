---
id: "0032"
title: RC Receiver Command MicroSD Blackbox Logging Architecture, 4-Byte Struct Alignment & Ring Buffer Snapshot Extraction
author: Drone Research Team / Developer
date: 2026-10-17
time: 02:40:00 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#blackbox", "#sdcard", "#rc_rx", "#ppm", "#freertos", "#alignment"]
references:
  - "[[0000-firmwareStructureLayerDecision]]"
  - "[[0001-taskConfigMiddlewareIsrDecision]]"
  - "[[0002-taskEnumsAndHalAbstractionDecision]]"
  - "[[0006-taskCreationConfigEnumsAndTaskArgsPassingDecision]]"
  - "[[0013-sdCardSpiStorageAndFlightLoggingArchitecture]]"
  - "[[0018-sdCardLoggingRestorationAndUserButtonShutdown]]"
  - "[[0019-universalFlightDataAnalyzerAndBinaryTelemetryAlignmentSpecification]]"
  - "[[0029-unificationOfInterTaskRingBufferSizeConstant]]"
  - "[[0030-rmtHardwarePpmDecodingFsIa6bAndBleTelemetryArchitecture]]"
  - "[[0031-sixGatePreflightTerminalDevelopmentAndTaskEncapsulationArchitecture]]"
---

# Zettel 0032: RC Receiver Command MicroSD Blackbox Logging Architecture, 4-Byte Struct Alignment & Ring Buffer Snapshot Extraction

## 1. Metadata & Scope
- **ID**: `0032`
- **Author**: Drone Research Team / Developer
- **Date**: 2026-10-17
- **Time**: 02:40:00 +07:00
- **Status**: Approved & Implemented
- **References**: `[Note 0000]`, `[Note 0001]`, `[Note 0002]`, `[Note 0006]`, `[Note 0013]`, `[Note 0018]`, `[Note 0019]`, `[Note 0029]`, `[Note 0030]`, `[Note 0031]`

---

## 2. Problem Statement & Motivation
In flight control systems, tuning cascaded PID attitude controllers, analyzing motor setpoint responses, and investigating flight anomalies requires correlating raw/filtered sensor reactions with actual pilot control inputs:
1. **Pilot Setpoint Blind Spot**: Prior to this implementation, the MicroSD blackbox flight logger recorded sensor IMU streams, magnetometer readings, barometric altitude, and power consumption, but omitted pilot command sticks (Roll, Pitch, Throttle, Yaw) and auxiliary flight mode switches.
2. **Deterministic Multi-Rate Coordination**: The RC receiver task operates at $50\,\text{Hz}$ (`TASK_RC_RECEIVER_FREQ_HZ`), while `sd_logger_task` executes at $100\,\text{Hz}$ (`TASK_SD_LOGGER_FREQ_HZ`). Logging must avoid redundant duplicate frame writes, sample starvation, or CPU-blocking inter-task synchronization.
3. **Strict 4-Byte ABI Memory Alignment**: As established in `[Note 0019]`, the 32-bit Xtensa LX7 ABI enforces natural 4-byte alignment for 32-bit types (`uint32_t`, `float`). Binary packet structures must include deliberate padding bytes following the 1-byte packet header tag to ensure predictable, corruption-free sector serialization.

---

## 3. Core Architectural Decisions

### Rule 1: Binary Packet Specification (`PktRc`) & 4-Byte Struct Alignment
In accordance with `[Note 0019]` and [`src/core/blackbox/sd_logger_task.cpp`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/core/blackbox/sd_logger_task.cpp), the binary RC packet structure is defined as:

```cpp
struct __attribute__((packed)) PktRc {
  uint8_t  type;          // Tag = 0x0A (PKT_TYPE_RC)
  uint32_t timestamp_us;  // Microsecond timestamp
  uint8_t  _pad[3];       // 3 explicit padding bytes for 4-byte float word alignment
  float    channels[6];   // 6 decoded channels (Roll, Pitch, Throttle, Yaw, Aux1, Aux2) in microseconds
};
static_assert(sizeof(PktRc) == 32, "PktRc layout must be strictly 32 bytes aligned");
```

- **Tag Byte**: `0x0A` (`PKT_TYPE_RC`).
- **Payload Size**: $4 + 3 + 24 = 31\,\text{bytes}$ (`<I 3x 6f`).
- **Total Disk Stride**: $1 + 31 = 32\,\text{bytes}$ ($32 \equiv 0 \pmod 4$), allowing exactly 16 RC packets to pack cleanly into a 512-byte SD card sector without boundary misalignment.

### Rule 2: Centralized Packet Type & Bitmask Registration
In [`include/config.h`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/include/config.h):
- `PKT_TYPE_RC = 0x0A` registered in `enum LogPacketType`.
- `LOG_MASK_RC = (1 << 9)` registered in `enum LogPacketBitmask`.
- Added to `ACTIVE_SD_LOG_PACKETS_MASK` multi-line bitmask interface:
  ```cpp
  constexpr uint16_t ACTIVE_SD_LOG_PACKETS_MASK =
      LOG_MASK_NONE
      | LOG_MASK_IMU_RAW
      | LOG_MASK_IMU_FILT
      | LOG_MASK_MAG_RAW
      | LOG_MASK_MAG_FILT
      | LOG_MASK_BARO_RAW
      | LOG_MASK_BARO_FILT
      // | LOG_MASK_ATT_QUAT
      // | LOG_MASK_ATT_EULER
      // | LOG_MASK_POWER
      // | LOG_MASK_RC
      ;
  ```

### Rule 3: Lock-Free Snapshot Extraction & Timestamp Tracking
`sd_logger_task` accesses `rc_data` and `rc_timestamp` using the thread-safe `get_snapshot()` API:
1. `rc_timestamp.get_snapshot(ts_snap)` and `rc_data.get_snapshot(rc_snap)` retrieve newly queued frames from the Layer 3 IPC ring buffer without locking `rc_receiver_task`.
2. A dedicated `ChannelTracker trk_rc` ensures monotonic timestamp ordering:
   $$\Delta t = t_{\text{curr}} - t_{\text{last}} > 0$$
   Frames are written only when a new timestamp arrives, eliminating duplicate writes during the 100 Hz logger task cycles.
3. Decoded channels ($1\text{--}6$) are mapped directly; missing or unpopulated auxiliary channels default safely to neutral $1500.0\,\mu\text{s}$.

---

## 4. Architectural Summary

| Layer | Component | Function |
|---|---|---|
| **Layer 1 / HAL** | `src/platforms/targets/espressif/pulse_rmt.cpp` | RMT Channel 4 (RX) PPM decoding with zero interrupt jitter |
| **Layer 2 / Driver** | `src/drivers/rc_rx/fs_ia6b.h/.cpp` | Validates pulse frames ($800\text{--}2200\,\mu\text{s}$) and detects failsafe |
| **Layer 3 / IPC** | `src/middleware/ipc.h/.cpp` | Lock-free `rc_data` and `rc_timestamp` ring buffers |
| **Layer 4 / Core** | `src/core/telemetry/rc_receiver_task.cpp` | 50 Hz task pushing valid decoded RC frames to IPC |
| **Layer 4 / Storage** | `src/core/blackbox/sd_logger_task.cpp` | 100 Hz task packaging `PktRc` (32B) into 512B sector buffers |
| **Layer 5 / Config** | `include/config.h` | `PKT_TYPE_RC = 0x0A`, `LOG_MASK_RC = (1 << 9)` |
