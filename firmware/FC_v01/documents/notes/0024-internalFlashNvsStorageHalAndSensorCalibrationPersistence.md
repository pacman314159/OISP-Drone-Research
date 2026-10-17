---
id: "0024"
title: Internal Flash NVS Storage HAL, FlashStorageDriver & Diagonal Soft-Iron SIMD Calibration Persistence Specification
author: Drone Research Team / Developer
date: 2026-10-13
time: 00:46:00 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#nvs", "#flash", "#hal", "#simd", "#calibration", "#ipc"]
references:
  - "[[0000-firmwareStructureLayerDecision]]"
  - "[[0001-taskConfigMiddlewareIsrDecision]]"
  - "[[0002-taskEnumsAndHalAbstractionDecision]]"
  - "[[0003-simdHalArchitectureAndLayeringRule]]"
  - "[[0006-taskCreationConfigEnumsAndTaskArgsPassingDecision]]"
  - "[[0008-lpfAndCalibrationTaskArchitecture]]"
  - "[[0013-sdCardSpiStorageAndFlightLoggingArchitecture]]"
  - "[[0016-coreDirectoryCompactionAndDomainConsolidation]]"
  - "[[0020-fsmBackbonePreFlightSensorChecksAndActiveCalibrationArchitecture]]"
  - "[[0023-fsmCentricCalibrationOrchestrationAndRgbLedArchitecture]]"
  - "[[0025-rotEllipsoidMagCalibSimdAdamEkfUnitSphereStackLifecycle]]"
  - "[[0026-sixPointAccelCalibSimdEllipsoidFitFsmLedArchitecture]]"
---

# Zettel 0024: Internal Flash NVS Storage HAL, FlashStorageDriver & Diagonal Soft-Iron SIMD Calibration Persistence Specification

## 1. Metadata & Scope
- **ID**: `0024`
- **Author**: Drone Research Team / Developer
- **Date**: 2026-10-13
- **Time**: 00:46:00 +07:00
- **Status**: Approved & Implemented
- **References**: `[Note 0000]`, `[Note 0002]`, `[Note 0003]`, `[Note 0008]`, `[Note 0013]`, `[Note 0016]`, `[Note 0020]`, `[Note 0023]`

---

## 2. Problem Statement & Architectural Requirements
High-reliability flight controller operation requires persistent zero-rate gyroscope offsets, 6-point accelerometer sensitivity matrices, and magnetometer hard/soft-iron distortion compensation across power cycles:
1. **Decoupled HAL Architecture**: Drivers across Layer 2 and algorithms across Layer 4 must remain strictly hardware-target agnostic per `[Note 0002]`. Concrete ESP-IDF `<nvs_flash.h>` calls must never leak into drivers or flight core tasks.
2. **Flash Wear & Safety Constraints**: Flash write and erase cycles are limited. Erasing $4\,\text{KB}$ sectors causes non-deterministic millisecond latency spikes ($10\text{--}50\,\text{ms}$) that stall the CPU instruction cache. Persistent writes must never occur within the $500\,\text{Hz}$ flight loop.
3. **Data Integrity & Schema Versioning**: Records saved to flash must feature magic number checks, schema versioning, and CRC32 verification to prevent booting with corrupted or obsolete parameters.
4. **Memory Compaction & Diagonal Soft-Iron Assumption**: Storing full $3 \times 3$ soft-iron correction matrices wastes valuable non-volatile storage slots on off-diagonal zeros under axis-aligned ellipsoid assumptions.
5. **Real-Time Vector Math Acceleration**: Calibration corrections must execute in nanoseconds without blocking real-time control loops.

---

## 3. Core Architectural Decisions

### Rule 1: Layer 1 Abstract Interface & Target Decoupling (`HAL_Flash`)
- Resides in `src/platforms/hal/hal_flash.h` as a pure virtual class:
  ```cpp
  class HAL_Flash {
  public:
    virtual ~HAL_Flash() = default;
    virtual bool init(const char* partition_name = "nvs") = 0;
    virtual bool read_blob(const char* ns_name, const char* key, void* out_data, size_t max_len, size_t* out_len) = 0;
    virtual bool write_blob(const char* ns_name, const char* key, const void* data, size_t len) = 0;
    virtual bool erase_key(const char* ns_name, const char* key) = 0;
    virtual bool erase_namespace(const char* ns_name) = 0;
  };
  HAL_Flash& get_flash_storage();
  ```
- Implemented for Espressif in `src/platforms/targets/espressif/flash.h` and `flash.cpp` (`ESP32_Flash`).
- Binds to the $16\,\text{KB}$ partition `"nvs"` at offset `0x9000` registered in `partitions.csv`.
- Automatically handles recovery from `ESP_ERR_NVS_NO_FREE_PAGES` and `ESP_ERR_NVS_NEW_VERSION_FOUND` by erasing the partition and reinitializing.
- Commits transactions atomically with `nvs_commit()`.

### Rule 2: Layer 2 Peripheral Driver (`FlashStorageDriver`)
- Resides in `src/drivers/storage/flash_storage_driver.h` and `flash_storage_driver.cpp`.
- Target-agnostic; consumes `HAL_Flash&` exclusively.
- Every record is prepended with a 12-byte `FlashRecordHeader`:
  ```cpp
  struct __attribute__((packed)) FlashRecordHeader {
    uint32_t magic;        // 0xCA11B001 (FLASH_RECORD_MAGIC)
    uint16_t schema_ver;   // 1
    uint16_t payload_len;  // Length of raw payload bytes
    uint32_t crc32;        // IEEE 802.3 CRC32 checksum over payload
  };
  ```
- Evaluates IEEE 802.3 polynomial (`0xEDB88320`) CRC32 over payload bytes. Corrupted or truncated entries are rejected on boot.

### Rule 3: Layer 3 Inter-Task Mutex Synchronization (`flash_mutex`)
- `flash_mutex` is declared in `src/middleware/ipc.h` and initialized in `init_ipc()` (`src/middleware/ipc.cpp`).
- All flash read/write transactions are protected by `flash_mutex` to eliminate concurrency race conditions between FSM calibration routines and future telemetry configuration tasks.

### Rule 4: Centralized IPC Calibration Struct & Memory Footprint
- Declared in `src/middleware/ipc.h`, instantiated in `src/middleware/ipc.cpp` (`calib_data`).
- In `SensorCalibrationData`, the magnetometer soft-iron matrix is modeled as a diagonal matrix $W_{soft} = \text{diag}(s_{mx}, s_{my}, s_{mz})$.
- Off-diagonal zeros are eliminated, reducing the struct to 64 bytes:
  ```cpp
  struct SensorCalibrationData {
    Vec3<float> gyro_bias{0.0f, 0.0f, 0.0f};   // Gyro zero-rate bias offsets (deg/s)
    Vec3<float> accel_bias{0.0f, 0.0f, 0.0f};  // Accel zero-g bias offsets (m/s^2)
    Vec3<float> accel_scale{1.0f, 1.0f, 1.0f}; // Accel 6-point sensitivity scaling
    Vec3<float> mag_bias{0.0f, 0.0f, 0.0f};    // Mag hard-iron bias offsets (Gauss)
    Vec3<float> mag_scale{1.0f, 1.0f, 1.0f};   // Mag soft-iron diagonal sensitivity scaling
    bool        is_valid{false};
  };

  extern SensorCalibrationData calib_data;
  extern FlashStorageDriver*   calib_storage;
  ```

### Rule 5: Hardware SIMD Hadamard Product Vector Acceleration
- Soft-iron matrix-vector multiplication simplifies to the element-wise Hadamard product:
  $$W_{soft} \cdot \vec{v} = \vec{s}_m \odot \vec{v}$$
- Implemented in `hal_simd::mul()` (`src/platforms/hal/hal_simd.h`, `targets/espressif/simd.cpp`) leveraging ESP32-S3 Xtensa PIE / SIMD instruction `dsps_mul_f32`.
- Exposed as `Matrix::hadamard()` in `src/core/math/matrix.h`.
- Sensor corrections in `calib.cpp` share symmetric SIMD pipelines:
  $$\vec{a}_{cal} = (\vec{a}_{raw} - \vec{b}_a) \odot \vec{s}_a$$
  $$\vec{m}_{cal} = (\vec{m}_{raw} - \vec{b}_m) \odot \vec{s}_m$$
  $$\vec{g}_{cal} = \vec{g}_{raw} - \vec{b}_g$$

### Rule 6: Flight Loop Isolation & Boot/Calibration Lifecycle
- In-flight preprocessing loops (`accel_gyro_preprocess_task`, `mag_preprocess_task`) strictly read calibrated parameters from fast SRAM via `calib_apply_*`, incurring zero flash latency.
- Flash writes occur exclusively when grounded:
  1. User triggers active calibration (`CALIB` state via user button held at boot).
  2. **Pre-Sampling Hands-Off Settling Period (`CALIB_SETTLING_DELAY_MS = 2000`)**: To prevent user button-release touch vibrations from corrupting zero-rate bias measurements, `task_calib_gyro` immediately triggers a fast amber flash burst (`LED_PATTERN_BLINK_FAST`, `PALETTE_AMBER`) and delays for $2.0\,\text{s}$.
  3. **Stationary Sampling Phase**: Once settling completes, the LED transitions to breathing cyan (`LED_PATTERN_BREATHE`, `PALETTE_CYAN`), collecting 400 stationary samples at $200\,\text{Hz}$ without mechanical interference.
  4. Calibration task completes sample collection and commits to NVS under `flash_mutex`.
  5. FSM receives completion notification and visualizes success via status LED (`PALETTE_GREEN`).
- On boot, `app_start()` reads calibration parameters from flash. If invalid or uncalibrated, safe default unity scaling is assigned and a warning is logged.

---

## 4. Traceability & Affected Files
- Abstract HAL: `src/platforms/hal/hal_flash.h`
- SIMD HAL: `src/platforms/hal/hal_simd.h`, `src/platforms/targets/espressif/simd.cpp`
- Vector Math: `src/core/math/matrix.h`
- Platform Target: `src/platforms/targets/espressif/flash.h`, `src/platforms/targets/espressif/flash.cpp`
- Peripheral Driver: `src/drivers/storage/flash_storage_driver.h`, `src/drivers/storage/flash_storage_driver.cpp`
- Middleware IPC: `src/middleware/ipc.h`, `src/middleware/ipc.cpp`
- Calibration Core: `src/core/sensor/calib.h`, `src/core/sensor/calib.cpp`
- Task Runners: `src/core/sensor/calib_tasks.cpp`, `src/core/sensor/sensor_preprocess_tasks.cpp`
- Active Calibration Specs: `[[0025-rotEllipsoidMagCalibSimdAdamEkfUnitSphereStackLifecycle]]`, `[[0026-sixPointAccelCalibSimdEllipsoidFitFsmLedArchitecture]]`
- Application Entry: `src/app/app.cpp`, `src/app/fsm.cpp`
- Partition Table: `partitions.csv`
