#pragma once
#include "core/math/matrix.h"
#include "config.h"

// =============================================================================
// STATE ESTIMATOR
// =============================================================================

class AttitudeEKF {
public:
  AttitudeEKF();

  void reset();
  void predict(float dt, const Vec3<float>& gyro_deg_s); // dt in s, gyro in deg/s
  void update_accel(const Vec3<float>& accel_g);          // accel in g
  void update_mag(const Vec3<float>& mag_unit);           // normalized unit vector

  Vec4 get_quaternion() const;
  Vec3<float> get_euler_angles() const;                  // Roll, Pitch, Yaw in degrees
  Vec3<float> get_gyro_bias() const;                     // rad/s
  const Vec7& get_state() const { return _x; }

private:
  void normalize_quaternion();

  Vec7                _x;       // State Vector [q0, q1, q2, q3, bx, by, bz] (quaternion + gyro biases in rad/s)
  Matrix<float, 7, 7> _P_cov;   // State error covariance matrix
  Matrix<float, 7, 7> _Q_cov;   // Process noise covariance matrix
  Matrix<float, 3, 3> _Ra_cov;  // Accelerometer measurement noise covariance matrix
  Matrix<float, 3, 3> _Rm_cov;  // Magnetometer measurement noise covariance matrix

  float _earth_bx;              // Earth reference magnetic field horizontal component (North)
  float _earth_bz;              // Earth reference magnetic field vertical component (Down)
};

// =============================================================================
// VERTICAL VELOCITY & ALTITUDE ESTIMATOR
// =============================================================================

class AltitudeEKF {
public:
  AltitudeEKF();

  void reset();
  void set_reference_pressure(float p0_pa);                                // Ground reference pressure P0 (Pa)
  float get_reference_pressure() const{ return _p0_pa; }                   // Pa

  void predict(float dt, const Vec3<float>& accel_g, const Vec4& quat);    // dt in s, accel in g, unit quat
  void update_baro(float pressure_pa);                                     // pressure in Pa

  float get_altitude() const{ return _x(0, 0); }                           // m (Z-Up AGL altitude above P0)
  float get_vertical_velocity() const{ return _x(1, 0); }                  // m/s (Z-Up climb rate)
  float get_accel_bias() const{ return _x(2, 0); }                         // m/s^2 (vertical accelerometer bias)
  float get_vertical_accel() const{ return _az_inertial; }                 // m/s^2 (bias-corrected inertial vertical accel)
  Vec3<float> get_output_vector() const{ return Vec3<float>(_x(0, 0), _x(1, 0), _az_inertial); }
  const Matrix<float, 3, 1>& get_state() const{ return _x; }

private:
  Matrix<float, 3, 1> _x;           // State vector [z (m), v_z (m/s), b_az (m/s^2)]
  Matrix<float, 3, 3> _P_cov;       // 3x3 State error covariance matrix
  Matrix<float, 3, 3> _Q_cov;       // 3x3 Process noise covariance matrix
  float               _R_baro;      // Barometer pressure measurement noise variance (Pa^2)
  float               _p0_pa;       // Ground reference pressure P0 (Pa)
  float               _az_inertial; // Latest bias-corrected inertial vertical acceleration (m/s^2)
};

