#pragma once
#include "core/math/matrix.h"
#include "config.h"

// =============================================================================
// LAYER 4 FLIGHT CORE & ALGORITHMS - STATE ESTIMATOR
// =============================================================================

class AttitudeEKF {
public:
  AttitudeEKF();

  void reset();
  void predict(float dt, const Vec3<float>& gyro_deg_s);
  void update_accel(const Vec3<float>& accel_g);
  void update_mag(const Vec3<float>& mag_unit);

  Vec4 get_quaternion() const;
  Vec3<float> get_euler_angles() const;
  Vec3<float> get_gyro_bias() const;
  const Vec7& get_state() const { return _x; }

private:
  void normalize_quaternion();

  Vec7                _x;       // State Vector [q0, q1, q2, q3, bx, by, bz] (quaternion + gyro biases in rad/s)
  Matrix<float, 7, 7> _P_cov;   // State error covariance matrix
  Matrix<float, 7, 7> _Q_cov;   // Process noise covariance matrix
  Matrix<float, 3, 3> _Ra_cov;  // Accelerometer measurement noise covariance matrix
  Matrix<float, 3, 3> _Rm_cov;  // Magnetometer measurement noise covariance matrix

  float _earth_bx; // Earth reference magnetic field horizontal component (North)
  float _earth_bz; // Earth reference magnetic field vertical component (Down)
};

extern AttitudeEKF g_ekf;

// Layer 4 FreeRTOS Task Runners
void ekf_roll_pitch_task(void* arg);
void ekf_yaw_task(void* arg);
