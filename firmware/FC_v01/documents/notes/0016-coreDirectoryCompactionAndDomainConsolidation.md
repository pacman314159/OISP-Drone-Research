---
id: "0016"
title: Layer 4 Core Directory Compaction, Submodule Consolidation & Task Renaming Specification
author: Drone Research Team / Developer
date: 2026-10-05
time: 21:40:00 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#refactoring", "#core", "#freertos"]
references:
  - "[[0000-firmwareStructureLayerDecision]]"
  - "[[0001-taskConfigMiddlewareIsrDecision]]"
  - "[[0002-taskEnumsAndHalAbstractionDecision]]"
  - "[[0006-taskCreationConfigEnumsAndTaskArgsPassingDecision]]"
  - "[[0008-lpfAndCalibrationTaskArchitecture]]"
  - "[[0009-distinctBleGattCharacteristicUuidIsolation]]"
  - "[[0010-bleMinimumPacketFrequencyAndDualTriggerFlush]]"
  - "[[0013-sdCardSpiStorageAndFlightLoggingArchitecture]]"
  - "[[0015-bleOtaArchitectureHardeningAndResourceCleanup]]"
  - "[[0017-bmp180TwoStepI2cTransactionAndBaroDaqArchitecture]]"
  - "[[0018-sdCardLoggingRestorationAndUserButtonShutdown]]"
  - "[[0038-osalLayer3IpcDecouplingAndInterfaceTasksConsolidation]]"
---

# Zettel 0016: Layer 4 Core Directory Compaction, Submodule Consolidation & Task Renaming Specification

## 1. Metadata & Context
- **ID**: `0016`
- **Author**: Drone Research Team / Developer
- **Date**: 2026-10-05
- **Time**: 21:40:00 +07:00
- **Status**: Approved & Enforced
- **References**: `[Note 0000]`, `[Note 0002]`, `[Note 0006]`, `[Note 0008]`, `[Note 0009]`, `[Note 0010]`, `[Note 0013]`, `[Note 0015]`, `[Note 0018]`

---

## 2. Problem Statement & Motivation
Prior to this decision, Layer 4 (`src/core/`) suffered from directory sprawl across 10 distinct subfolders (`daq`, `calib`, `control`, `estimators`, `filters`, `interface`, `logging`, `math`, `ota`, `telemetry`), several of which were empty or single-file placeholders. This fragmented the codebase, increased include path complexity, and blurred cohesive domain boundaries:
1. Signal processing algorithms were split artificially across `filters/` (`lpf.h`, `filter_tasks.cpp/.h`), `estimators/` (`est_tasks.cpp/.h`), and `math/` (`matrix.h`).
2. High-speed sensor sampling loops and calibration routines were bifurcated between `daq/` and `calib/`.
3. Communication transports were divided between `telemetry/` (`telem_tasks.cpp/.h`) and `ota/` (`ota_tasks.cpp/.h`), with filenames lacking radio protocol differentiation (critical for upcoming multi-radio support such as LoRa).
4. Flight data logging was generically named `logging/`, causing semantic ambiguity with UART serial logging rather than dedicated flight blackbox recording.

---

## 3. Core Architectural Decisions

### Rule 1: Consolidation into 6 Unified Functional Domains
Layer 4 (`src/core/`) is compacted into strictly 6 cohesive, non-overlapping functional subdirectories:

```
src/core/
├── sensor/     <- Unified high-frequency DAQ sampling, preprocessing & calibration routines
├── math/       <- Linear algebra, discrete LPF algorithms, ellipsoid fitting & estimators
├── telemetry/  <- Telemetry data packaging, transmission & wireless OTA
├── blackbox/   <- High-rate flight data logging & MicroSD storage
├── control/    <- Cascaded PID controllers & motor mixer
└── interface/  <- User button & RGB LED hardware interaction abstractions
```

1. **`sensor/`** (unified from `daq/` and `calib/`):
   - Houses high-frequency sensor acquisition loops (`daq_tasks.cpp`, `daq_tasks.h`), sensor preprocessing task runners (`sensor_preprocess_tasks.cpp`, `sensor_preprocess_tasks.h`), and calibration routines (`calib.cpp/.h`, `calib_tasks.cpp/.h`).
2. **`math/`** (incorporates `filters/` and `estimators/`):
   - Houses linear algebra (`matrix.h`), discrete 1st-order signal filters (`lpf.h`), 3D ellipsoid fitting (`ellipsoid_fit.cpp/.h`), mathematical state estimators (`estimators.cpp/.h`), RTOS estimator task runners (`estimator_tasks.cpp/.h`), and coordinate conversions.
3. **`telemetry/`** (incorporates `ota/`):
   - Unifies all wireless/wireline remote data streaming and over-the-air firmware update task runners (`ble_telem_tasks.cpp/.h`, `ble_ota_tasks.cpp/.h`).
4. **`blackbox/`** (renamed from `logging/`):
   - Dedicated exclusively to flight recording, high-rate sensor/state snapshot serialization, and MicroSD binary logging (`sd_logger_task.cpp`, `sd_logger_task.h`).
5. **`control/`**:
   - Dedicated to cascaded PID attitude/rate flight controllers and motor mixers (`control_tasks.cpp`).
6. **`interface/`**:
   - Preserved for UI indicators, status LEDs (WS2812 encapsulation), and user push buttons.

### Rule 2: Protocol-Explicit & Domain Task Naming
To future-proof the communication stack for multiple transceivers (e.g. BLE, LoRa, ESP-NOW) and clarify algorithm roles:
1. `ota_tasks.cpp/.h` $\rightarrow$ `ble_ota_tasks.cpp/.h`
2. `telem_tasks.cpp/.h` $\rightarrow$ `ble_telem_tasks.cpp/.h` (differentiating BLE telemetry from future `lora_telem_tasks.cpp/.h`)
3. `est_tasks.cpp/.h` $\rightarrow$ `estimator_tasks.cpp/.h`

### Rule 3: Single Source of Truth Includes
In `src/app/app.cpp`, task runners are included with clean domain-scoped paths:
```cpp
#include "core/sensor/daq_tasks.h"
#include "core/sensor/sensor_preprocess_tasks.h"
#include "core/telemetry/ble_telem_tasks.h"
#include "core/blackbox/sd_logger_task.h"
#include "core/telemetry/ble_ota_tasks.h"
```

---

## 4. Cross-References & Traceability
- Supersedes the 10-folder directory layout in `[[0000-firmwareStructureLayerDecision]]`.
- Amends submodule scoping in `[[0008-lpfAndCalibrationTaskArchitecture]]`.
- Updates task mapping paths in `[[0009-distinctBleGattCharacteristicUuidIsolation]]` and `[[0010-bleMinimumPacketFrequencyAndDualTriggerFlush]]`.
- Updates storage runner paths in `[[0013-sdCardSpiStorageAndFlightLoggingArchitecture]]` and `[[0018-sdCardLoggingRestorationAndUserButtonShutdown]]`.
- Updates OTA task identity paths in `[[0015-bleOtaArchitectureHardeningAndResourceCleanup]]`.
- Concrete Layer 4 math estimators and tasks implemented in `[[0032-dualCoreEkfStateEstimationAttitudeRingBufferArchitecture]]` (`estimators.h/.cpp` and `estimator_tasks.h/.cpp`).
