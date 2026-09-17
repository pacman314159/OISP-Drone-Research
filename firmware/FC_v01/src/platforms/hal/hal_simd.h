#pragma once
#include <cstdint>
#include <cstddef>
#include "config.h"

namespace hal_simd {

  // Floating-point (float) SIMD primitives
  float dot_product(const float* a, const float* b, size_t count);
  void add(const float* a, const float* b, float* out, size_t count);
  void sub(const float* a, const float* b, float* out, size_t count);
  void scale(const float* in, float scalar, float* out, size_t count);
  void mat_mul_f32(const float* A, const float* B, float* C, uint8_t m, uint8_t n, uint8_t k);
  bool inv3x3_f32(const float M[9], float Out[9]);

  // 16-bit Signed Integer (int16_t) ESP32-S3 Packed SIMD primitives
  int16_t dot_product(const int16_t* a, const int16_t* b, size_t count);
  void add(const int16_t* a, const int16_t* b, int16_t* out, size_t count);
  void sub(const int16_t* a, const int16_t* b, int16_t* out, size_t count);
  void scale(const int16_t* in, int16_t scalar, int16_t* out, size_t count);

  // 8-bit Signed Integer (int8_t) ESP32-S3 Packed SIMD primitives
  int8_t dot_product(const int8_t* a, const int8_t* b, size_t count);
  void add(const int8_t* a, const int8_t* b, int8_t* out, size_t count);
  void sub(const int8_t* a, const int8_t* b, int8_t* out, size_t count);
  void scale(const int8_t* in, int8_t scalar, int8_t* out, size_t count);

} // namespace hal_simd
