---
id: "0019"
title: Universal Flight Data Analyzer Architecture, Binary Telemetry 4-Byte Struct Alignment & Offline Multi-Rate Signal Processing Pipeline
author: Drone Research Team / Developer
date: 2026-10-08
time: 16:20:00 +07:00
tags: ["#zettelkasten", "#analyzers", "#firmware", "#architecture", "#fc_v01", "#fc_v02", "#blackbox", "#signal_processing", "#fft", "#struct_alignment"]
references:
  - "[[0000-firmwareStructureLayerDecision]]"
  - "[[0013-sdCardSpiStorageAndFlightLoggingArchitecture]]"
  - "[[0018-sdCardLoggingRestorationAndUserButtonShutdown]]"
---

# Zettel 0019: Universal Flight Data Analyzer Architecture, Binary Telemetry 4-Byte Struct Alignment & Offline Multi-Rate Signal Processing Pipeline

## 1. Metadata & Scope
- **ID**: `0019`
- **Author**: Drone Research Team / Developer
- **Date**: 2026-10-08
- **Time**: 16:20:00 +07:00
- **Status**: Approved & Implemented
- **References**: `[Note 0000]`, `[Note 0013]`, `[Note 0018]`

---

## 2. Problem Statement & Compiler Struct Alignment Discovery

### A. The Parsing Desynchronization Root Cause
Prior flight data inspection utilized fragmented standalone Python scripts (`baro_distribution.py`, `baro_fft.py`, `imu_distribution.py`, `imu_fft.py`, `mag_distribution.py`, `mag_fft.py`, `parse_flight_log.py`). In addition to maintenance overhead and manual code parameter editing, earlier parsers suffered from stream corruption and parsing desynchronization when processing raw binary flight logs (`LOG_XXXX.BIN`).

Investigation revealed that the firmware data structures in `src/core/blackbox/sd_logger_task.cpp` were declared without explicit packed attributes (`__attribute__((packed))`). On the 32-bit Xtensa LX7 architecture (ESP32-S3), GCC enforces **natural 4-byte alignment** for 32-bit integers and floating-point types (`Vec3<float>`, `Quaternion<float>`). Consequently, the compiler injects 3 padding bytes immediately following the 1-byte `type` field (`uint8_t`), resulting in 4-byte word alignment for subsequent fields:

| Packet Identifier | Firmware Enum | Theoretical Unpacked | True Memory Footprint (ESP32-S3 GCC ABI) | Struct Layout String |
|---|---|---|---|---|
| `PKT_TYPE_IMU_RAW` | `0x01` | 29 bytes | **32 bytes** | `<I 3x 6f` (`uint8_t type`, 3B pad, `uint32_t ts`, 6 $\times$ `float32`) |
| `PKT_TYPE_MAG_RAW` | `0x03` | 17 bytes | **20 bytes** | `<I 3x 3f` (`uint8_t type`, 3B pad, `uint32_t ts`, 3 $\times$ `float32`) |
| `PKT_TYPE_BARO_RAW` | `0x05` | 9 bytes | **12 bytes** | `<I 3x f` (`uint8_t type`, 3B pad, `uint32_t ts`, 1 $\times$ `float32`) |
| `PKT_TYPE_ATT_QUAT` | `0x07` | 21 bytes | **24 bytes** | `<I 3x 4f` (`uint8_t type`, 3B pad, `uint32_t ts`, 4 $\times$ `float32`) |
| `PKT_TYPE_ATT_EULER` | `0x08` | 17 bytes | **20 bytes** | `<I 3x 3f` (`uint8_t type`, 3B pad, `uint32_t ts`, 3 $\times$ `float32`) |

Legacy parsers reading unaligned byte lengths (e.g. 29 bytes instead of 32 bytes) shifted stream offsets by 3 bytes per packet, rapidly triggering false tag corruptions and sector dropouts. Accommodating the true 4-byte aligned ABI yields 100% data extraction integrity with zero packet drops.

---

## 3. Universal Flight Analyzer Architecture (`universal_flight_analyzer.py`)

All previous post-flight analysis routines have been consolidated into a single, modular, and interactive tool: [`analyzers/universal_flight_analyzer.py`](file:///d:/OneDrive_MSFT/Work/Drone_Research/analyzers/universal_flight_analyzer.py).

```
                      +---------------------------------------+
                      | Windows File Explorer (Native Dialog) |
                      +-------------------+-------------------+
                                          | Selects *.BIN log
                                          v
                      +---------------------------------------+
                      | Fast Pre-Scan & Channel Verification  |
                      +-------------------+-------------------+
                                          |
                                          v
                      +---------------------------------------+
                      |     Interactive Analysis CLI Menu     |
                      |   [Accel, Gyro, Mag, Baro] x [HIST, FFT] |
                      +-------------------+-------------------+
                                          | User Choice
                                          v
                      +---------------------------------------+
                      |   Transient Edge Trimming (80% Steady)|
                      +-------------------+-------------------+
                                          |
                 +------------------------+------------------------+
                 |                                                 |
                 v                                                 v
+----------------------------------+             +----------------------------------+
|  Time-Domain & Histogram Metrics  |             |  Spectral Analysis (FFT Window)  |
| - Mean (mu), StdDev (sigma)      |             | - Hanning Windowing              |
| - Min, Max, Range, 3-sigma Bounds|             | - Dominant Peak Frequency Detection|
| - Sampling Rate & Timing Jitter  |             | - Vibration Resonance Profiles   |
+-----------------+----------------+             +-----------------+----------------+
                  |                                                |
                  +------------------------+-----------------------+
                                           |
                                           v
                 +-------------------------------------------------+
                 |       Uniform Artifact Generation Pipeline      |
                 | - Dual-Stream Terminal Mirror: YYYY-LOG.txt     |
                 | - Auto-Increment 4-Digit Run ID (YYYY)          |
                 | - Standard Image Output: YYYY-XXXXX-<desc>.png  |
                 +-------------------------------------------------+
```

### A. OS Native GUI File Selection
- Built-in `tkinter.filedialog.askopenfilename` integration opens the native Windows File Explorer dialog on launch.
- Eliminates hardcoded file paths and the need to edit script variables across flight trials.

### B. Sector Padding & Header Deserialization
- Decodes the 32-byte `LogFileHeader` (`uint32_t magic = 0x474F4C44`, `version`, `start_time_us`, `reserved`).
- Automatically scans through 512-byte sectors, treating `0x00` padding bytes gracefully to maintain synchronization across block flush boundaries.

### C. Modular Multi-Channel Interactive Menu
- Pre-scans flight logs to detect available data channels.
- Presents an interactive numeric menu permitting individual, combined, or bulk selection:
  - **Accelerometer**: Raw 3-axis acceleration in $g$ (Earth gravity: $Z \approx 1.0\ g$, $X, Y \approx 0\ g$; scaled in firmware by $1 / 16384.0\text{ LSB}/g$).
  - **Gyroscope**: Raw 3-axis angular rates in $\text{deg/s}$ (scaled directly in firmware driver `mpu6050.cpp` by $1 / 131.0\text{ LSB}/(\text{deg/s})$; offline analyzers must **not** apply `np.degrees()`, preventing an erroneous $57.3\times$ inflation of uncalibrated zero-rate bias).
  - **Magnetometer**: Calibrated/raw 3-axis magnetic field in $\text{Gauss}$ (scaled in firmware by $1 / 1090.0\text{ LSB}/\text{Gauss}$).
  - **Barometer**: Atmospheric pressure in $\text{Pa}$ and barometric altitude in $\text{m}$ (hypsometric formula).

### D. Startup/Landing Transient Trimming
- Flight test data contains power-on stationary periods, hand launch oscillations, and ground collision transients.
- Automatic edge trimming rejects the initial 10% and terminal 10% samples (configurable), isolating the middle 80% steady flight regime for unbiased noise and vibration analysis.

### E. Distribution & Spectral Metrics
1. **Histogram & Time-Domain Analysis**:
   - Calculates sample count, true running frequency ($f_s$), and timing jitter ($\sigma_{\Delta t}$).
   - Computes statistical moments: $\mu$ (mean), $\sigma$ (standard deviation), min, max, dynamic range, and plots superimposed Gaussian fit curves with $\pm 3\sigma$ bounds.
2. **Fast Fourier Transform (FFT)**:
   - Removes DC bias, applies Hanning windowing to minimize spectral leakage, and computes single-sided amplitude spectra up to the Nyquist limit ($f_s / 2$).
   - Automatically detects and annotates the top 3 dominant harmonic peaks for motor/propeller vibration isolation.

### F. Dual-Stream Terminal Logging (`YYYY-LOG.txt`)
- All console outputs (file summaries, channel stats, frequency distributions, and peak detections) are captured via a dual-stream `TeeLogger`.
- Persistently saved to `YYYY-LOG.txt` both in `analyzers/` and `analyzers/img/` matching the run ID.

### G. Stateless Sequential Artifact Versioning
- Image and log artifacts strictly enforce the naming convention:
  ```text
  YYYY-XXXXX-<short_description>.png
  ```
  where:
  - `YYYY`: Unified 4-digit incremental session ID (all plots from a single run share the same ID).
  - `XXXXX`: Up to 5-character plot descriptor (`HIST`, `FFT`).
  - `<short_description>`: Concise identifier without spaces (e.g. `Accel_raw`, `Gyro_raw`, `Mag_raw`, `Baro_raw`).
- Zero external state tracking: Next run ID is determined dynamically by scanning `analyzers/img/` for `max(id) + 1` (starting at `0000`).

---

## 4. Legacy Tooling Deprecation & Cleanup

The following legacy scripts have been decommissioned and removed from `analyzers/`:
- `baro_distribution.py`, `baro_fft.py`
- `imu_distribution.py`, `imu_fft.py`
- `mag_distribution.py`, `mag_fft.py`, `mag_time_analysis.py`
- `parse_flight_log.py`

---

## 5. Cross-References & Traceability

- **Predecessor Note**: `[Note 0018]` restored physical MicroSD SPI logging and specified the binary file format.
- **Companion Script**: [`analyzers/universal_flight_analyzer.py`](file:///d:/OneDrive_MSFT/Work/Drone_Research/analyzers/universal_flight_analyzer.py) implements the full decoding and signal processing specification.
- **Data Target**: [`analyzers/binary_black_box_data/`](file:///d:/OneDrive_MSFT/Work/Drone_Research/analyzers/binary_black_box_data) contains flight logs (`LOG_XXXX.BIN`).
- **Artifacts Output**: [`analyzers/img/`](file:///d:/OneDrive_MSFT/Work/Drone_Research/analyzers/img) stores versioned distribution plots, FFT spectra, and session logs.
