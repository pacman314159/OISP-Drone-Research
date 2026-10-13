#include "core/math/ellipsoid_fit.h"
#include <cmath>

void ellipsoid_fit_init(EllipsoidFitState& state, const Vec3<float>* points, uint16_t n,
                        uint16_t max_epochs, float lr, float tol){
  if(points == nullptr || n < CALIB_MAG_MIN_SAMPLES) return;

  state.max_epochs    = max_epochs;
  state.lr            = lr;
  state.tol           = tol;
  state.current_epoch = 0;
  state.converged     = false;
  state.initialized   = true;

  const float inv_n = 1.0f / static_cast<float>(n);

  // 1. Initial Guess: Compute empirical centroid v0 (hard-iron offset)
  Vec3<float> sum{0.0f, 0.0f, 0.0f};
  for(uint16_t i = 0; i < n; ++i)
    sum += points[i];

  state.v0 = sum * inv_n;

  // 2. Initial Guess: Compute reference field magnitude B0 (mean radial distance)
  float sum_dist = 0.0f;
  for(uint16_t i = 0; i < n; ++i){
    Vec3<float> diff = points[i] - state.v0;
    sum_dist += diff.norm();
  }

  state.B0 = sum_dist * inv_n;
  if(state.B0 <= 1e-6f) state.B0 = 1.0f;

  // 3. Initial Guess: Soft-iron matrix W = (1 / B0) * I (maps initial sphere to unit sphere)
  state.W = Matrix<float, 3, 3>::identity() * (1.0f / state.B0);

  // 4. Initialize Adam momentum states to zero
  state.m_v0 = Vec3<float>(0.0f, 0.0f, 0.0f);
  state.v_v0 = Vec3<float>(0.0f, 0.0f, 0.0f);
  state.m_W  = Matrix<float, 3, 3>::zeros();
  state.v_W  = Matrix<float, 3, 3>::zeros();

  // 5. Compute pre-optimization RMSE on unit sphere residual
  float sum_r_sq = 0.0f;
  for(uint16_t i = 0; i < n; ++i){
    Vec3<float> d = points[i] - state.v0;
    Vec3<float> u = state.W * d;
    float r = u.norm_sq() - 1.0f;
    sum_r_sq += r * r;
  }

  state.initial_rmse = sqrtf(sum_r_sq * inv_n);
  state.current_rmse = state.initial_rmse;
  state.prev_rmse    = 1e9f;
}

bool ellipsoid_fit_step(EllipsoidFitState& state, const Vec3<float>* points, uint16_t n,
                        uint8_t epochs_in_batch){
  if(points == nullptr || n < CALIB_MAG_MIN_SAMPLES || !state.initialized) return true;

  constexpr float beta1 = 0.9f;
  constexpr float beta2 = 0.999f;
  constexpr float eps   = 1e-8f;
  const float n_f = static_cast<float>(n);

  for(uint8_t ep = 0; ep < epochs_in_batch; ++ep){
    if(state.converged || state.current_epoch >= state.max_epochs) return true;

    state.current_epoch++;
    state.prev_rmse = state.current_rmse;

    Vec3<float> s_u{0.0f, 0.0f, 0.0f};
    Matrix<float, 3, 3> G = Matrix<float, 3, 3>::zeros();
    float sum_r_sq = 0.0f;

    // Evaluate point cloud projections and residuals (accelerated via Matrix SIMD operators)
    for(uint16_t i = 0; i < n; ++i){
      Vec3<float> d = points[i] - state.v0;
      Vec3<float> u = state.W * d;

      float r = u.norm_sq() - 1.0f;
      sum_r_sq += r * r;

      Vec3<float> r_u = u * r;
      s_u += r_u;
      G += r_u * d.transpose();
    }

    state.current_rmse = sqrtf(sum_r_sq / n_f);

    // Early stopping tolerance check: |RMSE_t - RMSE_{t-1}| < tol
    if(state.current_epoch > 1){
      float rmse_diff = fabsf(state.current_rmse - state.prev_rmse);
      if(rmse_diff < state.tol){
        state.converged = true;
        return true;
      }
    }

    // Analytical Gradients
    // grad_v0 = -(4/n) * W * s_u
    Vec3<float> grad_v0 = (state.W * s_u) * (-4.0f / n_f);

    // grad_W = (2/n) * (G + G^T)
    Matrix<float, 3, 3> grad_W = (G + G.transpose()) * (2.0f / n_f);

    // Adam Momentum Updates
    float beta1_t = powf(beta1, static_cast<float>(state.current_epoch));
    float beta2_t = powf(beta2, static_cast<float>(state.current_epoch));
    float inv_one_minus_beta1_t = 1.0f / (1.0f - beta1_t);
    float inv_one_minus_beta2_t = 1.0f / (1.0f - beta2_t);

    // Update v0 momentum & position
    Vec3<float> grad_v0_sq = grad_v0.hadamard(grad_v0);
    state.m_v0 = state.m_v0 * beta1 + grad_v0 * (1.0f - beta1);
    state.v_v0 = state.v_v0 * beta2 + grad_v0_sq * (1.0f - beta2);

    for(uint8_t k = 0; k < 3; ++k){
      float m_hat = state.m_v0[k] * inv_one_minus_beta1_t;
      float v_hat = state.v_v0[k] * inv_one_minus_beta2_t;
      state.v0[k] -= state.lr * m_hat / (sqrtf(v_hat) + eps);
    }

    // Update W momentum & matrix
    Matrix<float, 3, 3> grad_W_sq = grad_W.hadamard(grad_W);
    state.m_W = state.m_W * beta1 + grad_W * (1.0f - beta1);
    state.v_W = state.v_W * beta2 + grad_W_sq * (1.0f - beta2);

    for(uint8_t k = 0; k < 9; ++k){
      float m_hat = state.m_W[k] * inv_one_minus_beta1_t;
      float v_hat = state.v_W[k] * inv_one_minus_beta2_t;
      state.W[k] -= state.lr * m_hat / (sqrtf(v_hat) + eps);
    }

    // Enforce exact matrix symmetry W = 0.5 * (W + W^T)
    state.W = (state.W + state.W.transpose()) * 0.5f;
  }

  return (state.converged || state.current_epoch >= state.max_epochs);
}

void ellipsoid_fit_get_params(const EllipsoidFitState& state, EllipsoidParameters& out_params){
  out_params.v0           = state.v0;
  out_params.W            = state.W;
  out_params.B0           = state.B0;
  out_params.initial_rmse = state.initial_rmse;
  out_params.final_rmse   = state.current_rmse;
  out_params.epochs_run   = state.current_epoch;
  out_params.converged    = state.converged;
}
