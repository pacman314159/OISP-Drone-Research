#pragma once
#include <cstdint>
#include <cmath>
#include "config.h"
#include "core/math/matrix.h"

// =============================================================================
// LAYER 4 MATHEMATICAL MODULE: 3D ROTATED ELLIPSOID FIT (SIMD ADAM)
// =============================================================================

struct EllipsoidParameters {
  Vec3<float>         v0;           // Hard-iron bias center offset v0 (Gauss)
  Matrix<float, 3, 3> W;            // Soft-iron symmetric matrix W (maps directly to unit sphere)
  float               B0;           // Geomagnetic reference field magnitude (Gauss)
  float               initial_rmse; // Pre-optimization RMSE on unit sphere residual
  float               final_rmse;   // Post-optimization converged RMSE
  uint16_t            epochs_run;   // Number of optimization epochs computed
  bool                converged;    // True if early stopping tolerance was satisfied
};

struct EllipsoidFitState {
  Vec3<float>         v0;           // Center offset estimate (Gauss)
  Matrix<float, 3, 3> W;            // Soft-iron matrix estimate (unit sphere)
  float               B0;           // Reference field magnitude (Gauss)

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

// Backward-compatibility alias
using EllipsoidOptimizerState = EllipsoidFitState;

// Ellipsoid fitting lifecycle routines
void ellipsoid_fit_init(EllipsoidFitState& state, const Vec3<float>* points, uint16_t n,
                        uint16_t max_epochs = CALIB_MAG_MAX_EPOCHS,
                        float lr = CALIB_MAG_ADAM_LR,
                        float tol = CALIB_MAG_ADAM_TOL);

bool ellipsoid_fit_step(EllipsoidFitState& state, const Vec3<float>* points, uint16_t n,
                        uint8_t epochs_in_batch);

void ellipsoid_fit_get_params(const EllipsoidFitState& state, EllipsoidParameters& out_params);

// Backward-compatibility inline wrappers
inline void ellipsoid_optimizer_init(EllipsoidFitState& state, const Vec3<float>* points, uint16_t n,
                                     uint16_t max_epochs = CALIB_MAG_MAX_EPOCHS,
                                     float lr = CALIB_MAG_ADAM_LR,
                                     float tol = CALIB_MAG_ADAM_TOL){
  ellipsoid_fit_init(state, points, n, max_epochs, lr, tol);
}

inline bool ellipsoid_optimizer_step(EllipsoidFitState& state, const Vec3<float>* points, uint16_t n,
                                     uint8_t epochs_in_batch){
  return ellipsoid_fit_step(state, points, n, epochs_in_batch);
}

inline void ellipsoid_optimizer_get_params(const EllipsoidFitState& state, EllipsoidParameters& out_params){
  ellipsoid_fit_get_params(state, out_params);
}
