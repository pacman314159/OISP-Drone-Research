#pragma once

#include <cmath>
#include "config.h"
#include "src/core/math/matrix.h"
#include "platforms/hal/hal_simd.h"
#include <Arduino.h>

#ifndef TWO_PI
#define TWO_PI 2.0f * 3.1415926535897932384626433832795
#endif

// =============================================================================
// LAYER 4 CORE: 1ST-ORDER DISCRETE LOW-PASS FILTER (TUSTIN BILINEAR TRANSFORM)
// =============================================================================
// Transfer function in s-plane: H(s) = omega_c / (s + omega_c)
// Discretized via Tustin formula: s = (2/T) * (z - 1) / (z + 1)
// Difference Equation: y[i] = A * (x[i] + x[i-1]) - B * y[i-1]
// Where:
//   T = 1 / sample_freq_hz (seconds)
//   omega_c = 2 * PI * cutoff_freq_hz (rad/s)
//   A = (T * omega_c) / (T * omega_c + 2.0f)
//   B = (T * omega_c - 2.0f) / (T * omega_c + 2.0f)
// =============================================================================

struct LowPassFilter1stOrder {
  float a;            // LPF input gain coefficient A
  float b;            // LPF feedback gain coefficient B
  Vec3<float> x_prev; // Previous input vector x[i-1]
  Vec3<float> y_prev; // Previous output vector y[i-1]
  bool initialized;   // First sample initialization flag
};

inline void lpf_init(LowPassFilter1stOrder* filter, float sample_freq_hz, float cutoff_freq_hz){
  if(filter == nullptr) return;

  filter->x_prev = Vec3<float>{0.0f, 0.0f, 0.0f};
  filter->y_prev = Vec3<float>{0.0f, 0.0f, 0.0f};
  filter->initialized = false;

  if(sample_freq_hz <= 0.0f || cutoff_freq_hz <= 0.0f){
    filter->a = 1.0f;
    filter->b = 0.0f;
    return;
  }

  const float dt = 1.0f / sample_freq_hz; // seconds
  const float omega_c = TWO_PI * cutoff_freq_hz; // rad/s
  const float dt_omega = dt * omega_c;

  filter->a = dt_omega / (dt_omega + 2.0f);
  filter->b = (dt_omega - 2.0f) / (dt_omega + 2.0f);
}

inline Vec3<float> lpf_apply(LowPassFilter1stOrder* filter, const Vec3<float>& x_curr){
  if(filter == nullptr) return x_curr;

  if(!filter->initialized){
    filter->x_prev = x_curr;
    filter->y_prev = x_curr;
    filter->initialized = true;
    return x_curr;
  }

  Vec3<float> y_curr;

#if (!ENABLE_SIMD_ACCELERATION)
  // C++ Portable Scalar fallback block
  for(uint8_t i = 0; i < 3; ++i)
    y_curr.m[i] = filter->a * (x_curr.m[i] + filter->x_prev.m[i]) - filter->b * filter->y_prev.m[i];
#else
  // Target Hardware SIMD Accelerated Block (ESP32-S3 ESP-DSP)
  Vec3<float> sum_x;
  Vec3<float> term_a;
  Vec3<float> term_b;

  hal_simd::add(x_curr.m, filter->x_prev.m, sum_x.m, 3);
  hal_simd::scale(sum_x.m, filter->a, term_a.m, 3);
  hal_simd::scale(filter->y_prev.m, filter->b, term_b.m, 3);
  hal_simd::sub(term_a.m, term_b.m, y_curr.m, 3);
#endif

  filter->x_prev = x_curr;
  filter->y_prev = y_curr;

  return y_curr;
}
