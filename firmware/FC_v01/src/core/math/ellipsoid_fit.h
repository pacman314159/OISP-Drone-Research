#pragma once
#include <cstdint>
#include "config.h"
#include "core/math/matrix.h"

// =============================================================================
// LAYER 4 MATHEMATICAL MODULE: 3D ELLIPSOID FIT (SIMD ADAM)
// =============================================================================

// -----------------------------------------------------------------------------
// 1. 9-Parameter Full Rotated Ellipsoid Fit (Hard-Iron Bias + 3x3 Soft-Iron Matrix)
// -----------------------------------------------------------------------------
struct Ellipsoid9ParamResult {
  Vec3<float>         v0;           // Hard-iron bias center offset v0 (Gauss / sensor units)
  Matrix<float, 3, 3> W;            // Soft-iron symmetric matrix W (maps directly to unit sphere)
  float               b0;           // Geomagnetic reference field magnitude (Gauss)
  float               initial_rmse; // Pre-optimization RMSE on unit sphere residual
  float               final_rmse;   // Post-optimization converged RMSE
  uint16_t            epochs_run;   // Number of optimization epochs computed
  bool                converged;    // True if early stopping tolerance was satisfied
};

struct Ellipsoid9ParamState {
  Vec3<float>         v0;           // Center offset estimate
  Matrix<float, 3, 3> W;            // Soft-iron matrix estimate (unit sphere)
  float               b0;           // Reference field magnitude

  // Adam optimizer 1st and 2nd momentum states
  Vec3<float>         m_v0;
  Vec3<float>         v_v0;
  Matrix<float, 3, 3> m_W;
  Matrix<float, 3, 3> v_W;

  float    prev_rmse;
  float    current_rmse;
  float    initial_rmse;
  uint16_t current_epoch;
  uint16_t max_epochs;
  float    lr;
  float    tol;
  bool     converged;
  bool     initialized;
};

// 9-Parameter Rotated Ellipsoid Lifecycle Routines
void ellipsoid_fit_9param_init(Ellipsoid9ParamState& state, const Vec3<float>* points, uint16_t n,
                               uint16_t max_epochs = CALIB_MAG_MAX_EPOCHS,
                               float lr = CALIB_MAG_ADAM_LR,
                               float tol = CALIB_MAG_ADAM_TOL,
                               uint16_t min_samples = 9);

bool ellipsoid_fit_9param_step(Ellipsoid9ParamState& state, const Vec3<float>* points, uint16_t n,
                               uint8_t epochs_in_batch,
                               uint16_t min_samples = 9);

void ellipsoid_fit_9param_get_params(const Ellipsoid9ParamState& state, Ellipsoid9ParamResult& out_params);

// -----------------------------------------------------------------------------
// 2. 6-Parameter Axis-Aligned Ellipsoid Fit (Zero-G Bias + 3-Axis Scale Vector)
// -----------------------------------------------------------------------------
struct Ellipsoid6ParamResult {
  Vec3<float> v0;           // Zero-g bias center offset v0 (g / sensor units)
  Vec3<float> s;            // Sensitivity scale vector s (maps directly to unit sphere)
  float       b0;           // Reference field magnitude (mean radial distance)
  float       initial_rmse; // Pre-optimization RMSE on unit sphere residual
  float       final_rmse;   // Post-optimization converged RMSE
  uint16_t    epochs_run;   // Number of optimization epochs computed
  bool        converged;    // True if early stopping tolerance was satisfied
};

struct Ellipsoid6ParamState {
  Vec3<float> v0;           // Center offset estimate
  Vec3<float> s;            // Sensitivity scale estimate
  float       b0;           // Reference field magnitude

  // Adam optimizer 1st and 2nd momentum states
  Vec3<float> m_v0;
  Vec3<float> v_v0;
  Vec3<float> m_s;
  Vec3<float> v_s;

  float    prev_rmse;
  float    current_rmse;
  float    initial_rmse;
  uint16_t current_epoch;
  uint16_t max_epochs;
  float    lr;
  float    tol;
  bool     converged;
  bool     initialized;
};

// 6-Parameter Axis-Aligned Ellipsoid Lifecycle Routines
void ellipsoid_fit_6param_init(Ellipsoid6ParamState& state, const Vec3<float>* points, uint16_t n,
                               uint16_t max_epochs = CALIB_ACCEL_MAX_EPOCHS,
                               float lr = CALIB_ACCEL_ADAM_LR,
                               float tol = CALIB_ACCEL_ADAM_TOL,
                               uint16_t min_samples = 6);

bool ellipsoid_fit_6param_step(Ellipsoid6ParamState& state, const Vec3<float>* points, uint16_t n,
                               uint8_t epochs_in_batch,
                               uint16_t min_samples = 6);

void ellipsoid_fit_6param_get_params(const Ellipsoid6ParamState& state, Ellipsoid6ParamResult& out_params);
