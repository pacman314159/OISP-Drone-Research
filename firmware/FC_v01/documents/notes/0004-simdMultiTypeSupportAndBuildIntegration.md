---
id: "0004"
title: Multi-Type SIMD Hardware Acceleration (float, int16_t, int8_t) & ESP-DSP Build Integration
author: Drone Research Team / Developer
date: 2026-09-17
time: 18:36:00 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#simd", "#espressif", "#espdsp"]
references:
  - "[[0000-firmwareStructureLayerDecision]]"
  - "[[0002-taskEnumsAndHalAbstractionDecision]]"
  - "[[0003-simdHalArchitectureAndLayeringRule]]"
---

# Zettel 0004: Multi-Type SIMD Hardware Acceleration (float, int16_t, int8_t) & ESP-DSP Build Integration

## 1. Metadata & Context
- **ID**: `0004`
- **Author**: Drone Research Team / Developer
- **Date**: 2026-09-17
- **Time**: 18:36:00 +07:00
- **Status**: Approved & Implemented
- **References**: `[Note 0000]`, `[Note 0002]`, `[Note 0003]`

---

## 2. Executive Summary of Changes

During this iteration, the `FC_v01` flight controller math sub-system and hardware abstraction layer (HAL) were refactored to support multi-type packed SIMD vector acceleration (`float`, `int16_t`, `int8_t`) on the ESP32-S3 microcontroller, enforce Layer 1 hardware purity, re-structure preprocessor math blocks in `matrix.h`, and resolve ESP-DSP component build dependencies.

---

## 3. Key Architecture & Design Implementations

### 3.1 Unified `Matrix<T, ROWS, COLS>` Class Architecture
- **Single Template Matrix Class**: Removed deprecated `types.h` and `IMUSample` structs. Replaced matrix abstractions with a single `Matrix<T, ROWS, COLS>` template class in `src/core/math/matrix.h`.
- **`Vec3<T>` Inheritance & Anonymous Union**: `Vec3<T>` inherits `Matrix<T, 3, 1>` with an anonymous union (`union { T m[ROWS * COLS]; struct { T x, y, z; }; }`), allowing both array indexing `m[i]` for SIMD buffer passing and vector member access (`x, y, z`).

### 3.2 Layer 1 Pure Target HAL (`hal_simd`)
Following the rules set in **Zettel 0003**:
- **Hardware Purity**: `src/platforms/hal/hal_simd.h` and `src/platforms/targets/espressif/simd.cpp` contain strictly genuine hardware SIMD primitives without `#else` fallbacks or non-SIMD scalar functions (e.g., `biquad_f32` was removed from Layer 1).
- **Multi-Type Hardware SIMD Support**:
  - **32-Bit Float (`float`)**: Accelerated via ESP-DSP `dsps_dotprod_f32`, `dsps_add_f32`, `dsps_sub_f32`, `dsps_mulc_f32`, and matrix multiplication `dspm_mult_f32`.
  - **16-Bit Signed Integer (`int16_t`)**: Accelerated via 128-bit packed SIMD instructions `dsps_dotprod_s16`, `dsps_add_s16`, `dsps_sub_s16`, and `dsps_mulc_s16`.
  - **8-Bit Signed Integer (`int8_t`)**: Accelerated via 128-bit packed SIMD instructions `dsps_dp_s8`, `dsps_add_s8`, `dsps_sub_s8`, and element-wise scaling.

### 3.3 Re-Ordering of Preprocessor Math Blocks in Layer 4 (`matrix.h`)
- Structured `matrix.h` around `#if (!ENABLE_SIMD_ACCELERATION)`:
  - **Top Block (`#if (!ENABLE_SIMD_ACCELERATION)`)**: Conventional C++ scalar loops for portability and non-SIMD execution.
  - **Bottom Block (`#else`)**: Hardware SIMD accelerated execution block with `if constexpr` type dispatching for `float`, `int16_t`, and `int8_t`.

### 3.4 ESP-DSP Component & Build System Integration
- **ESP-IDF Component Manager**: Registered `espressif/esp-dsp: "^1.4.0"` in `src/idf_component.yml` and added `REQUIRES esp-dsp` to `src/CMakeLists.txt`.
- **Header Standardization**: Included `#include "esp_dsp.h"` in `simd.cpp` to pull in Espressif DSP vector and matrix prototypes cleanly.
- **PlatformIO Configuration**: Added `-Wno-error=implicit-function-declaration` to `build_flags` in `platformio.ini` and cleaned up git `lib_deps` to prevent PlatformIO from trying to compile demo applications inside `esp-dsp`.
- **Driver Fix**: Resolved reference type binding mismatch in `daq_tasks.cpp` (`temp_raw` changed from `float` to `int16_t`).

---

## 4. Coding Style Compliance
- **Single-Line Control Flow Bracing**: Omitted curly braces `{}` for single-line statements inside `if`, `else`, `while`, `for`, and `switch`.
- **Function Declaration Bracing**: Enforced `void func(){` (no space before `{`).
- **Control Flow Keyword & Opening Brace Spacing**: Enforced `if(condition){` and `}else{`.
- **Namespace Indentation**: Indented declarations/definitions inside `namespace` blocks by 2 spaces.
