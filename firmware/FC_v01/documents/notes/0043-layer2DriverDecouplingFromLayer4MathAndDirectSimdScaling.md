---
id: "0043"
title: Layer 2 Driver Decoupling from Layer 4 Math and Direct Layer 1 SIMD Scaling Architecture
author: Drone Research Team / Developer
date: 2026-10-28
time: 01:05:00 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#layers", "#drivers", "#simd", "#dsp", "#clean-code"]
references:
  - "[[0000-firmwareStructureLayerDecision]]"
  - "[[0042-layer5ExclusiveTaskSuspensionAuthorityAndOtaDecoupling]]"
---

# Zettel 0043: Layer 2 Driver Decoupling from Layer 4 Math and Direct Layer 1 SIMD Scaling Architecture

## 1. Metadata & Scope
- **ID**: `0043`
- **Author**: Drone Research Team / Developer
- **Date**: 2026-10-28
- **Time**: 01:05:00 +07:00
- **Status**: Approved & Enforced Architectural Decision
- **Module Context**: Layer 2 Drivers (`src/drivers/imu/mpu6050.h`, `src/drivers/imu/mpu6050.cpp`, `src/drivers/imu/hmc5883l.h`, `src/drivers/imu/hmc5883l.cpp`), Layer 1 HAL SIMD (`src/platforms/hal/hal_simd.h`), Layer 4 DAQ (`src/core/sensor/daq_tasks.cpp`)

---

## 2. Problem Statement: Upward Dependency Inversion (Layer 2 $\rightarrow$ Layer 4)
Under prior revisions, `mpu6050.h` and `hmc5883l.h` directly included `"src/core/math/matrix.h"` and accepted references to `Vec3<int16_t>&` and `Vec3<float>&` in their core acquisition methods (`get_all_data_raw` and `get_all_data`).

This created a severe architectural layering violation:
1. **Upward Dependency Inversion**:
   Per the 5-layer system architecture (`documents/notes/0000-firmwareStructureLayerDecision.md`), **Layer 2 (Device Drivers)** must strictly depend downward on **Layer 1 (HAL / Platform Primitives)** and **Layer 0 (Hardware / Config)**. Layer 2 must never import headers from **Layer 4 (Domain Flight Algorithms & Mathematical Libraries)**.
2. **False Coupling Root Cause**:
   The inclusion of `matrix.h` inside IMU drivers was not driven by complex matrix manipulation, but solely to utilize `operator*=(float scalar)` on `Vec3<float>` to trigger hardware-accelerated SIMD vector multiplication during LSB-to-physical unit conversion.

---

## 3. Core Architectural Decision: Primitive Array Interface with Direct Layer 1 SIMD

### 3.1 Driver Primitive Buffer Interface
Layer 2 hardware drivers must interface with callers using primitive fixed-size arrays (`int16_t[3]` and `float[3]`) or raw buffer pointers, avoiding any dependency on Layer 4 mathematical structs:

```cpp
// src/drivers/imu/mpu6050.h (Layer 2)
#include "config.h"
#include "platforms/hal/hal_simd.h" // Layer 1 HAL SIMD primitive dependency

class MPU6050 {
public:
  bool get_all_data_raw(int16_t accel_raw_msb[3], int16_t gyro_raw_msb[3], int16_t& temp_raw_msb);
  bool get_all_data(float accel[3], float gyro[3], float& temp);
  ...
};
```

```cpp
// src/drivers/imu/hmc5883l.h (Layer 2)
#include "config.h"
#include "platforms/hal/hal_simd.h"

class HMC5883L {
public:
  bool get_all_data_raw(int16_t mag_raw_msb[3]);
  bool get_all_data(float mag[3]);
  ...
};
```

### 3.2 In-Place Layer 1 Hardware SIMD Scaling
Physical conversion of raw integers to physical units ($\text{m/s}^2$, $\text{rad/s}$, $\text{Gauss}$) now invokes Layer 1 `hal_simd::scale()` (`dsps_mulc_f32`) directly inside the driver methods:

```cpp
// src/drivers/imu/mpu6050.cpp
bool MPU6050::get_all_data(float accel[3], float gyro[3], float& temp){
  if(accel == nullptr || gyro == nullptr) return false;
  int16_t accel_raw[3], gyro_raw[3];
  int16_t temp_raw = 0;
  if(!get_all_data_raw(accel_raw, gyro_raw, temp_raw)) return false;

  accel[0] = static_cast<float>(accel_raw[0]);
  accel[1] = static_cast<float>(accel_raw[1]);
  accel[2] = static_cast<float>(accel_raw[2]);
#if (ENABLE_SIMD_ACCELERATION)
  hal_simd::scale(accel, _inv_accel_sens, accel, 3);
#else
  accel[0] *= _inv_accel_sens;
  accel[1] *= _inv_accel_sens;
  accel[2] *= _inv_accel_sens;
#endif

  gyro[0] = static_cast<float>(gyro_raw[0]);
  gyro[1] = static_cast<float>(gyro_raw[1]);
  gyro[2] = static_cast<float>(gyro_raw[2]);
#if (ENABLE_SIMD_ACCELERATION)
  hal_simd::scale(gyro, _inv_gyro_sens, gyro, 3);
#else
  gyro[0] *= _inv_gyro_sens;
  gyro[1] *= _inv_gyro_sens;
  gyro[2] *= _inv_gyro_sens;
#endif

  temp = (static_cast<float>(temp_raw) / 340.0f) + 36.53f;
  return true;
}
```

### 3.3 Zero-Copy Layer 4 Integration
Layer 4 tasks (`src/core/sensor/daq_tasks.cpp`) pass contiguous memory pointers of their local `Vec3<float>` instances directly into the drivers:

```cpp
// src/core/sensor/daq_tasks.cpp (Layer 4)
Vec3<float> accel, gyro;
float temp = 0.0f;
...
bool success = mpu->get_all_data(accel.m, gyro.m, temp);
...
Vec3<float> mag;
bool success = hmc->get_all_data(mag.m);
```

This guarantees:
1. **Strict Layer Boundary**: Layer 2 drivers depend solely on Layer 1 (`platforms/hal/hal_simd.h`) and Layer 0 (`config.h`), with zero knowledge of Layer 4.
2. **Zero Allocation & Zero Overhead**: Pointers directly reference the underlying `float m[3]` union array of `Vec3<float>`, eliminating copies and intermediate stack allocations.
3. **Preserved SIMD Throughput**: Hardware DSP acceleration (`dsps_mulc_f32`) remains 100% active.
