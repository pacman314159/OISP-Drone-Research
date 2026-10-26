#include "estimators.h"
#include <cmath>
#include "config.h"

AttitudeEKF::AttitudeEKF(){
  _earth_bx = EKF_EARTH_BX;
  _earth_bz = EKF_EARTH_BZ;

  _Q_cov = Matrix<float, 7, 7>::zeros();
  for(uint8_t i = 0; i < 4; ++i) _Q_cov(i, i) = EKF_DEFAULT_Q_QUAT_VAR;
  for(uint8_t i = 4; i < 7; ++i) _Q_cov(i, i) = EKF_DEFAULT_Q_BIAS_VAR;

  _Ra_cov = Matrix<float, 3, 3>::zeros();
  _Ra_cov(0, 0) = EKF_DEFAULT_R_ACCEL_X + EKF_DEFAULT_R_ACCEL;
  _Ra_cov(1, 1) = EKF_DEFAULT_R_ACCEL_Y + EKF_DEFAULT_R_ACCEL;
  _Ra_cov(2, 2) = EKF_DEFAULT_R_ACCEL_Z + EKF_DEFAULT_R_ACCEL;

  _Rm_cov = Matrix<float, 3, 3>::zeros();
  _Rm_cov(0, 0) = EKF_DEFAULT_R_MAG_X + EKF_DEFAULT_R_MAG;
  _Rm_cov(1, 1) = EKF_DEFAULT_R_MAG_Y + EKF_DEFAULT_R_MAG;
  _Rm_cov(2, 2) = EKF_DEFAULT_R_MAG_Z + EKF_DEFAULT_R_MAG;

  reset();
}

void AttitudeEKF::reset(){
  _x = Matrix<float, 7, 1>::zeros();
  _x(0, 0) = 1.0f; // Unit quaternion [q0, q1, q2, q3] = [1, 0, 0, 0], zero gyro biases
  _P_cov = Matrix<float, 7, 7>::identity() * 0.01f;
}

void AttitudeEKF::normalize_quaternion(){
  Vec4 q{_x(0, 0), _x(1, 0), _x(2, 0), _x(3, 0)};
  q.normalize(); // Accelerated via hal_simd::dot_product and hal_simd::scale in matrix.h
  _x(0, 0) = q[0];
  _x(1, 0) = q[1];
  _x(2, 0) = q[2];
  _x(3, 0) = q[3];
}

void AttitudeEKF::predict(float dt, const Vec3<float>& gyro_deg_s){
  float q0 = _x(0, 0), q1 = _x(1, 0), q2 = _x(2, 0), q3 = _x(3, 0);
  float dt2 = dt * 0.5f;

  // Convert angular rates to rad/s using DEG_TO_RAD (MATH_PI / 180.0f) and subtract estimated gyro biases
  float wx = (gyro_deg_s.x * DEG_TO_RAD) - _x(4, 0);
  float wy = (gyro_deg_s.y * DEG_TO_RAD) - _x(5, 0);
  float wz = (gyro_deg_s.z * DEG_TO_RAD) - _x(6, 0);

  // 1. State Vector Propagation via SIMD Matrix Multiplication (hal_simd::mat_mul_f32)
  // Discrete kinematic transition: q_{k+1} = (I_{4x4} + 0.5 * dt * Omega(w)) * q_k
  Matrix<float, 4, 4> A = Matrix<float, 4, 4>::identity();
  A(0, 1) = -dt2 * wx; A(0, 2) = -dt2 * wy; A(0, 3) = -dt2 * wz;
  A(1, 0) =  dt2 * wx; A(1, 2) =  dt2 * wz; A(1, 3) = -dt2 * wy;
  A(2, 0) =  dt2 * wy; A(2, 1) = -dt2 * wz; A(2, 3) =  dt2 * wx;
  A(3, 0) =  dt2 * wz; A(3, 1) =  dt2 * wy; A(3, 2) = -dt2 * wx;

  Vec4 q{q0, q1, q2, q3};
  Vec4 q_pred = A * q; // Accelerated via hal_simd::mat_mul_f32 in matrix.h
  _x(0, 0) = q_pred[0];
  _x(1, 0) = q_pred[1];
  _x(2, 0) = q_pred[2];
  _x(3, 0) = q_pred[3];

  // 2. State Transition Jacobian Matrix F_mat (7x7)
  Matrix<float, 7, 7> F_mat = Matrix<float, 7, 7>::identity();
  F_mat(0, 1) = -dt2 * wx; F_mat(0, 2) = -dt2 * wy; F_mat(0, 3) = -dt2 * wz; F_mat(0, 4) =  dt2 * q1; F_mat(0, 5) =  dt2 * q2; F_mat(0, 6) =  dt2 * q3;
  F_mat(1, 0) =  dt2 * wx; F_mat(1, 2) =  dt2 * wz; F_mat(1, 3) = -dt2 * wy; F_mat(1, 4) = -dt2 * q0; F_mat(1, 5) =  dt2 * q3; F_mat(1, 6) = -dt2 * q2;
  F_mat(2, 0) =  dt2 * wy; F_mat(2, 1) = -dt2 * wz; F_mat(2, 3) =  dt2 * wx; F_mat(2, 4) = -dt2 * q3; F_mat(2, 5) = -dt2 * q0; F_mat(2, 6) =  dt2 * q1;
  F_mat(3, 0) =  dt2 * wz; F_mat(3, 1) =  dt2 * wy; F_mat(3, 2) = -dt2 * wx; F_mat(3, 4) =  dt2 * q2; F_mat(3, 5) = -dt2 * q1; F_mat(3, 6) = -dt2 * q0;

  // 3. Error Covariance Propagation via SIMD: P = F * P * F^T + Q
  _P_cov = (F_mat * _P_cov * F_mat.transpose()) + _Q_cov;
}

void AttitudeEKF::update_accel(const Vec3<float>& accel_g){
  Vec3<float> z_a = accel_g;
  z_a.normalize();

  float q0 = _x(0, 0), q1 = _x(1, 0), q2 = _x(2, 0), q3 = _x(3, 0);

  // 1. Observation Model: Expected Gravity Vector in Body Frame
  Vec3<float> h_a(
    2.0f * (q1 * q3 - q0 * q2),
    2.0f * (q0 * q1 + q2 * q3),
    q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3
  );

  // 2. Accelerometer Measurement Jacobian Ha (3x7)
  Matrix<float, 3, 7> Ha = Matrix<float, 3, 7>::zeros();
  Ha(0, 0) = -2.0f * q2; Ha(0, 1) =  2.0f * q3; Ha(0, 2) = -2.0f * q0; Ha(0, 3) =  2.0f * q1;
  Ha(1, 0) =  2.0f * q1; Ha(1, 1) =  2.0f * q0; Ha(1, 2) =  2.0f * q3; Ha(1, 3) =  2.0f * q2;
  Ha(2, 0) =  2.0f * q0; Ha(2, 1) = -2.0f * q1; Ha(2, 2) = -2.0f * q2; Ha(2, 3) =  2.0f * q3;

  // 3. Innovation & Gain: S = Ha * P * Ha^T + Ra, K = P * Ha^T * S^-1
  Vec3<float> y_a = z_a - h_a;
  Matrix<float, 7, 3> Ha_T = Ha.transpose();
  Matrix<float, 3, 3> S = (Ha * _P_cov * Ha_T) + _Ra_cov;
  Matrix<float, 7, 3> K = _P_cov * Ha_T * S.inverse3x3();

  // 4. State & Covariance Update
  Matrix<float, 7, 1> delta_x = K * static_cast<const Matrix<float, 3, 1>&>(y_a);
  _x += delta_x;
  _P_cov = (Matrix<float, 7, 7>::identity() - (K * Ha)) * _P_cov;

  normalize_quaternion();
}

void AttitudeEKF::update_mag(const Vec3<float>& mag_unit){
  float q0 = _x(0, 0), q1 = _x(1, 0), q2 = _x(2, 0), q3 = _x(3, 0);
  float bx = _earth_bx, bz = _earth_bz;

  // 1. Observation Model: Expected Magnetic Field Vector in Body Frame
  Vec3<float> h_m(
    bx * (q0 * q0 + q1 * q1 - q2 * q2 - q3 * q3) + 2.0f * bz * (q1 * q3 - q0 * q2),
    2.0f * bx * (q1 * q2 - q0 * q3) + 2.0f * bz * (q0 * q1 + q2 * q3),
    2.0f * bx * (q0 * q2 + q1 * q3) + bz * (q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3)
  );

  // 2. Magnetometer Measurement Jacobian Hm (3x7)
  Matrix<float, 3, 7> Hm = Matrix<float, 3, 7>::zeros();
  Hm(0, 0) =  2.0f * bx * q0 - 2.0f * bz * q2; Hm(0, 1) =  2.0f * bx * q1 + 2.0f * bz * q3; Hm(0, 2) = -2.0f * bx * q2 - 2.0f * bz * q0; Hm(0, 3) = -2.0f * bx * q3 + 2.0f * bz * q1;
  Hm(1, 0) = -2.0f * bx * q3 + 2.0f * bz * q1; Hm(1, 1) =  2.0f * bx * q2 + 2.0f * bz * q0; Hm(1, 2) =  2.0f * bx * q1 + 2.0f * bz * q3; Hm(1, 3) = -2.0f * bx * q0 + 2.0f * bz * q2;
  Hm(2, 0) =  2.0f * bx * q2 + 2.0f * bz * q0; Hm(2, 1) =  2.0f * bx * q3 - 2.0f * bz * q1; Hm(2, 2) =  2.0f * bx * q0 - 2.0f * bz * q2; Hm(2, 3) =  2.0f * bx * q1 + 2.0f * bz * q3;

  // 3. Innovation & Gain: S = Hm * P * Hm^T + Rm, K = P * Hm^T * S^-1
  Vec3<float> y_m = mag_unit - h_m;
  Matrix<float, 7, 3> Hm_T = Hm.transpose();
  Matrix<float, 3, 3> S = (Hm * _P_cov * Hm_T) + _Rm_cov;
  Matrix<float, 7, 3> K = _P_cov * Hm_T * S.inverse3x3();

  // 4. State & Covariance Update
  Matrix<float, 7, 1> delta_x = K * static_cast<const Matrix<float, 3, 1>&>(y_m);
  _x += delta_x;
  _P_cov = (Matrix<float, 7, 7>::identity() - (K * Hm)) * _P_cov;

  normalize_quaternion();
}

Vec4 AttitudeEKF::get_quaternion() const {
  Vec4 q;
  q[0] = _x(0, 0);
  q[1] = _x(1, 0);
  q[2] = _x(2, 0);
  q[3] = _x(3, 0);
  return q;
}

Vec3<float> AttitudeEKF::get_euler_angles() const {
  float q0 = _x(0, 0), q1 = _x(1, 0), q2 = _x(2, 0), q3 = _x(3, 0);

  // Roll (phi): rotation about body X-axis (-180 to +180 deg)
  float roll = atan2f(2.0f * (q0 * q1 + q2 * q3), q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3) * RAD_TO_DEG;

  // Pitch (theta): rotation about body Y-axis (-90 to +90 deg, clamped)
  float sin_pitch = -2.0f * (q1 * q3 - q0 * q2);
  if(sin_pitch > 1.0f) sin_pitch = 1.0f;
  if(sin_pitch < -1.0f) sin_pitch = -1.0f;
  float pitch = asinf(sin_pitch) * RAD_TO_DEG;

  // Yaw (psi): rotation about body Z-axis (-180 to +180 deg)
  float yaw = atan2f(2.0f * (q1 * q2 + q0 * q3), q0 * q0 + q1 * q1 - q2 * q2 - q3 * q3) * RAD_TO_DEG;

  return Vec3<float>(roll, pitch, yaw);
}

Vec3<float> AttitudeEKF::get_gyro_bias() const {
  return Vec3<float>(_x(4, 0), _x(5, 0), _x(6, 0));
}

// =============================================================================
// LAYER 4 FLIGHT CORE & ALGORITHMS - VERTICAL VELOCITY & ALTITUDE ESTIMATOR
// =============================================================================

AltitudeEKF::AltitudeEKF(){
  _p0_pa = BARO_SEA_LEVEL_PRESSURE_PA;
  _R_baro = EKF_ALT_DEFAULT_R_BARO_PA_VAR + EKF_ALT_DEFAULT_R_BARO_MARGIN;

  _Q_cov = Matrix<float, 3, 3>::zeros();
  _Q_cov(0, 0) = EKF_ALT_DEFAULT_Q_POS_VAR;
  _Q_cov(1, 1) = EKF_ALT_DEFAULT_Q_VEL_VAR;
  _Q_cov(2, 2) = EKF_ALT_DEFAULT_Q_BIAS_VAR;

  reset();
}

void AltitudeEKF::reset(){
  _x = Matrix<float, 3, 1>::zeros();
  _P_cov = Matrix<float, 3, 3>::identity() * 0.1f;
  _P_cov(2, 2) = 0.01f;
  _az_inertial = 0.0f;
}

void AltitudeEKF::set_reference_pressure(float p0_pa){
  if(p0_pa < SENSOR_STREAM_BARO_MIN_PA || p0_pa > SENSOR_STREAM_BARO_MAX_PA) return;
  _p0_pa = p0_pa;
  _x(0, 0) = 0.0f; // Reset altitude z = 0.0 m AGL
  _x(1, 0) = 0.0f; // Reset vertical velocity vz = 0.0 m/s
  _P_cov(0, 0) = 0.1f;
  _P_cov(1, 1) = 0.1f;
  _P_cov(0, 1) = _P_cov(1, 0) = 0.0f;
}

void AltitudeEKF::predict(float dt, const Vec3<float>& accel_g, const Vec4& quat){
  if(dt <= 0.0f || dt > 0.1f) return;

  float q0 = quat[0], q1 = quat[1], q2 = quat[2], q3 = quat[3];

  // Direction of inertial Z axis in body frame: r3(q) = [2(q1*q3 - q0*q2), 2(q0*q1 + q2*q3), q0^2 - q1^2 - q2^2 + q3^2]^T
  Vec3<float> r3(
    2.0f * (q1 * q3 - q0 * q2),
    2.0f * (q0 * q1 + q2 * q3),
    q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3
  );

  // Kinematic upward vertical acceleration (m/s^2) in Z-Up convention
  float az_proj_g = r3.dot(accel_g);
  float az_raw = (az_proj_g - 1.0f) * STANDARD_GRAVITY_MSS;

  // Bias-corrected inertial vertical acceleration: a_z_inertial = a_z_raw - b_a,z
  _az_inertial = az_raw - _x(2, 0);

  // 1. Kinematic state propagation
  float dt2 = 0.5f * dt * dt;
  _x(0, 0) += (_x(1, 0) * dt) + (_az_inertial * dt2);
  _x(1, 0) += _az_inertial * dt;
  // _x(2, 0) accelerometer bias remains constant in prediction

  // 2. State transition Jacobian matrix F (3x3)
  Matrix<float, 3, 3> F_mat = Matrix<float, 3, 3>::identity();
  F_mat(0, 1) = dt;
  F_mat(0, 2) = -dt2;
  F_mat(1, 2) = -dt;

  // 3. Error covariance propagation (matrix.h handles SIMD acceleration)
  _P_cov = (F_mat * _P_cov * F_mat.transpose()) + _Q_cov;
}

void AltitudeEKF::update_baro(float pressure_pa){
  if(pressure_pa < SENSOR_STREAM_BARO_MIN_PA || pressure_pa > SENSOR_STREAM_BARO_MAX_PA) return;

  // Guard altitude ratio for numerical stability in barometric formula
  float ratio = 1.0f - (_x(0, 0) / 44330.0f);
  if(ratio < 0.1f) ratio = 0.1f;

  // 1. Nonlinear observation model: h(x) = P0 * (1 - z/44330)^5.255
  float h_baro = _p0_pa * powf(ratio, 5.255f);

  // 2. Analytical measurement Jacobian H = [H0, 0, 0]:
  // H0 = dh/dz = -(5.255 * P0 / 44330) * (1 - z/44330)^4.255
  float H0 = -(5.255f * _p0_pa / 44330.0f) * powf(ratio, 4.255f);

  // 3. Scalar innovation & innovation covariance S = H0^2 * P00 + R_baro
  float y = pressure_pa - h_baro;
  float S = (H0 * H0 * _P_cov(0, 0)) + _R_baro;
  if(S < 1e-6f) return;

  // 4. Kalman gain vector K = P * H^T * S^-1 = (H0 / S) * [P00, P10, P20]^T
  float H0_div_S = H0 / S;
  float K0 = H0_div_S * _P_cov(0, 0);
  float K1 = H0_div_S * _P_cov(1, 0);
  float K2 = H0_div_S * _P_cov(2, 0);

  // 5. State update: x = x + K * y
  _x(0, 0) += K0 * y;
  _x(1, 0) += K1 * y;
  _x(2, 0) += K2 * y;

  // Accelerometer bias sanity clamp [-2.0, +2.0] m/s^2
  if(_x(2, 0) > 2.0f) _x(2, 0) = 2.0f;
  if(_x(2, 0) < -2.0f) _x(2, 0) = -2.0f;

  // 6. Covariance update: P = (I - K * H) * P
  Matrix<float, 3, 3> I_KH = Matrix<float, 3, 3>::identity();
  I_KH(0, 0) -= K0 * H0;
  I_KH(1, 0) -= K1 * H0;
  I_KH(2, 0) -= K2 * H0;

  _P_cov = I_KH * _P_cov;

  // Symmetrize covariance matrix
  _P_cov(0, 1) = _P_cov(1, 0) = 0.5f * (_P_cov(0, 1) + _P_cov(1, 0));
  _P_cov(0, 2) = _P_cov(2, 0) = 0.5f * (_P_cov(0, 2) + _P_cov(2, 0));
  _P_cov(1, 2) = _P_cov(2, 1) = 0.5f * (_P_cov(1, 2) + _P_cov(2, 1));
}

