---
id: "0003"
title: Layer 1 SIMD HAL Architecture & Zero-Fallback Rule
author: Drone Research Team / Developer
date: 2026-09-17
time: 15:40:00 +07:00
tags:
  - #zettelkasten
  - #firmware
  - #architecture
  - #fc_v01
  - #simd
  - #hal
references:
  - "[[0000-firmwareStructureLayerDecision]]"
  - "[[0002-taskEnumsAndHalAbstractionDecision]]"
---

# Zettel 0003: Layer 1 SIMD HAL Architecture & Zero-Fallback Rule

## 1. Metadata & Context
- **ID**: `0003`
- **Author**: Drone Research Team / Developer
- **Date**: 2026-09-17
- **Time**: 15:40:00 +07:00
- **Status**: Approved & Enforced
- **References**: `[Note 0000]`, `[Note 0002]`

---

## 2. Core Architectural Decision: Layer 1 Pure Target HAL

### Rule 1: Layer 1 Drivers & HAL Have Zero Preprocessor Fallbacks
1. **No Fallback `#else` Blocks in Layer 1 Target Code**:
   - Hardware target implementation files under `src/platforms/targets/<target>/` (e.g., `espressif/simd.cpp` or `stm/simd.cpp`) MUST provide raw, direct, hardware-accelerated target implementations without checking whether a feature macro is enabled or providing fallback `#else` blocks.
   - Layer 1 target files serve strictly as physical hardware execution drivers.

2. **Core Layer (Layer 4) Manages Compilation Flags & Fallbacks**:
   - The decision to invoke SIMD acceleration (via `ENABLE_SIMD_ACCELERATION`) or fallback to standard C++ loops belongs strictly to Layer 4 (`src/core/math/`) and higher algorithm layers.
   - Higher layers check system configuration options (`config.h`) and dispatch calls accordingly, keeping Layer 1 HAL modules clean, decoupled, and free from feature preprocessor clutter.

### Rule 2: `hal_simd` Hardware-Only Philosophy
1. **`hal_simd` Contains ONLY Target SIMD Routines**:
   - `hal_simd` module in Layer 1 is strictly for genuine target hardware SIMD routines (e.g. `float` 32-bit, `int16_t` 16-bit, and `int8_t` 8-bit packed SIMD on ESP32-S3).
   - `hal_simd` MUST NOT include dummy scalar fallbacks for data types that lack hardware SIMD support (such as `int32_t`).
2. **Layer 4 (`matrix.h`) Handles Type Fallbacks**:
   - Types that have hardware SIMD support (`float`, `int16_t`, `int8_t`) route to `hal_simd`.
   - Types that do NOT have target SIMD hardware support (such as `int32_t` or `double`) fall back to scalar C++ loops inside Layer 4 (`matrix.h`).

---

## 3. Impact & Architectural Rules
1. **Layer 1 Purity**: Never put `#else` scalar fallback routines inside `src/platforms/targets/`.
2. **SIMD-Only Scope**: `hal_simd` provides overloaded target SIMD functions strictly for SIMD-capable types (`float`, `int16_t`, and `int8_t`).
3. **Layer 4 Control**: Wrap feature selection and non-SIMD type fallback logic inside core math routines in Layer 4 (`matrix.h`).
