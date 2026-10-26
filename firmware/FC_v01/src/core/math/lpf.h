#pragma once

#include <cmath>
#include "config.h"
#include "matrix.h"
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

  // Leverages Vec3<T> (Matrix<T, 3, 1>) overloaded operators with internal SIMD acceleration
  Vec3<float> y_curr = (x_curr + filter->x_prev) * filter->a - filter->y_prev * filter->b;

  filter->x_prev = x_curr;
  filter->y_prev = y_curr;

  return y_curr;
}

// =============================================================================
// LAYER 4 CORE: 2ND-ORDER DISCRETE LOW-PASS FILTER (TUSTIN BILINEAR TRANSFORM)
// =============================================================================
// Transfer function in s-plane: H(s) = omega_n^2 / (s^2 + 2*zeta*omega_n*s + omega_n^2)
// Discretized via Tustin formula: s = (2/T) * (z - 1) / (z + 1), with K = T * omega_n
// Difference Equation:
//   y[i] = A * (x[i] + 2*x[i-1] + x[i-2]) - B1 * y[i-1] - B2 * y[i-2]
// Where:
//   T = 1 / sample_freq_hz (seconds)
//   omega_n = 2 * PI * cutoff_freq_hz (rad/s)
//   K = T * omega_n (dimensionless)
//   D0 = K^2 + 4*zeta*K + 4.0f
//   A  = K^2 / D0
//   B1 = 2.0f * (K^2 - 4.0f) / D0
//   B2 = (K^2 - 4*zeta*K + 4.0f) / D0
// =============================================================================

constexpr float BUTTERWORTH_ZETA = 0.70710678f; // 1 / sqrt(2) (dimensionless damping ratio)

class LowPassFilter2ndOrder {
public:
  LowPassFilter2ndOrder(){
    reset();
    _a = 1.0f;
    _b1 = 0.0f;
    _b2 = 0.0f;
  }

  LowPassFilter2ndOrder(float sample_freq_hz, float cutoff_freq_hz, float zeta){
    init(sample_freq_hz, cutoff_freq_hz, zeta);
  }

  void reset(){
    _x_prev1 = Vec3<float>{0.0f, 0.0f, 0.0f};
    _x_prev2 = Vec3<float>{0.0f, 0.0f, 0.0f};
    _y_prev1 = Vec3<float>{0.0f, 0.0f, 0.0f};
    _y_prev2 = Vec3<float>{0.0f, 0.0f, 0.0f};
    _initialized = false;
  }

  void init(float sample_freq_hz, float cutoff_freq_hz, float zeta){
    reset();

    if(sample_freq_hz <= 0.0f || cutoff_freq_hz <= 0.0f || zeta <= 0.0f){
      _a = 1.0f;
      _b1 = 0.0f;
      _b2 = 0.0f;
      return;
    }

    const float dt = 1.0f / sample_freq_hz;          // seconds
    const float omega_n = TWO_PI * cutoff_freq_hz;   // rad/s
    const float k = dt * omega_n;                    // dimensionless
    const float k2 = k * k;                          // dimensionless
    const float four_zeta_k = 4.0f * zeta * k;       // dimensionless
    const float d0 = k2 + four_zeta_k + 4.0f;        // dimensionless

    _a = k2 / d0;
    _b1 = (2.0f * (k2 - 4.0f)) / d0;
    _b2 = (k2 - four_zeta_k + 4.0f) / d0;
  }

  Vec3<float> apply(const Vec3<float>& x_curr){
    if(!_initialized){
      _x_prev1 = x_curr;
      _x_prev2 = x_curr;
      _y_prev1 = x_curr;
      _y_prev2 = x_curr;
      _initialized = true;
      return x_curr;
    }

    // Leverages Vec3<T> (Matrix<T, 3, 1>) overloaded operators with internal SIMD acceleration
    Vec3<float> y_curr = (x_curr + _x_prev1 * 2.0f + _x_prev2) * _a - _y_prev1 * _b1 - _y_prev2 * _b2;

    _x_prev2 = _x_prev1;
    _x_prev1 = x_curr;
    _y_prev2 = _y_prev1;
    _y_prev1 = y_curr;

    return y_curr;
  }

protected:
  float _a;             // Numerator feedforward gain coefficient A (dimensionless)
  float _b1;            // 1st-lag feedback gain coefficient B1 (dimensionless)
  float _b2;            // 2nd-lag feedback gain coefficient B2 (dimensionless)
  Vec3<float> _x_prev1; // Previous input vector x[i-1] (sensor units)
  Vec3<float> _x_prev2; // Two-step previous input vector x[i-2] (sensor units)
  Vec3<float> _y_prev1; // Previous output vector y[i-1] (sensor units)
  Vec3<float> _y_prev2; // Two-step previous output vector y[i-2] (sensor units)
  bool _initialized;    // First-sample transient suppression flag
};

// =============================================================================
// LAYER 4 CORE: 2ND-ORDER BUTTERWORTH LOW-PASS FILTER (zeta = sqrt(2)/2)
// =============================================================================
class ButterworthFilter : public LowPassFilter2ndOrder {
public:
  ButterworthFilter() : LowPassFilter2ndOrder(){}

  ButterworthFilter(float sample_freq_hz, float cutoff_freq_hz)
    : LowPassFilter2ndOrder(sample_freq_hz, cutoff_freq_hz, BUTTERWORTH_ZETA){}

  void init(float sample_freq_hz, float cutoff_freq_hz){
    LowPassFilter2ndOrder::init(sample_freq_hz, cutoff_freq_hz, BUTTERWORTH_ZETA);
  }
};

inline void lpf_init(LowPassFilter2ndOrder* filter, float sample_freq_hz, float cutoff_freq_hz, float zeta){
  if(filter == nullptr) return;
  filter->init(sample_freq_hz, cutoff_freq_hz, zeta);
}

inline void lpf_init(ButterworthFilter* filter, float sample_freq_hz, float cutoff_freq_hz){
  if(filter == nullptr) return;
  filter->init(sample_freq_hz, cutoff_freq_hz);
}

inline Vec3<float> lpf_apply(LowPassFilter2ndOrder* filter, const Vec3<float>& x_curr){
  if(filter == nullptr) return x_curr;
  return filter->apply(x_curr);
}
