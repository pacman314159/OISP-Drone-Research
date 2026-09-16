#pragma once

#include <cstdint>
#include <cstring>
#include <cmath>
#include <initializer_list>

template<uint8_t ROWS, uint8_t COLS>
class Matrix {
public:
  float m[ROWS * COLS];

  // Default constructor does nothing to prevent double-writing 0s for speed.
  Matrix() = default;

  // Initializer list constructor (e.g., Vec7 x = {1, 0, 0, 0, 0, 0, 0};)
  Matrix(std::initializer_list<float> values){
    uint8_t i = 0;
    for(float val : values)
      if(i < ROWS * COLS) m[i++] = val;
  }

  static Matrix<ROWS, COLS> zeros(){
    Matrix<ROWS, COLS> Z;
    memset(Z.m, 0, sizeof(Z.m));
    return Z;
  }

  static Matrix<ROWS, COLS> identity(){
    static_assert(ROWS == COLS, "Identity only valid for square matrices");
    Matrix<ROWS, COLS> I = zeros();
    for(uint8_t i = 0; i < ROWS; i++){
      I.m[i * COLS + i] = 1.0f;
    }
    return I;
  }

  // Access operator: M(row, col) - zero indexed
  float& operator()(uint8_t row, uint8_t col){
    return m[row * COLS + col];
  }

  const float& operator()(uint8_t row, uint8_t col) const{
    return m[row * COLS + col];
  }

  Matrix<ROWS, COLS>& operator+=(const Matrix<ROWS, COLS>& B){
    for(uint8_t i = 0; i < ROWS * COLS; ++i) this->m[i] += B.m[i];
    return *this;
  }

  Matrix<ROWS, COLS>& operator-=(const Matrix<ROWS, COLS>& B){
    for(uint8_t i = 0; i < ROWS * COLS; ++i) this->m[i] -= B.m[i];
    return *this;
  }

  Matrix<ROWS, COLS>& operator*=(float scalar){
    for(uint8_t i = 0; i < ROWS * COLS; ++i) this->m[i] *= scalar;
    return *this;
  }

  Matrix<ROWS, COLS> operator+(const Matrix<ROWS, COLS>& B) const{
    Matrix<ROWS, COLS> result = *this;
    result += B;
    return result;
  }

  Matrix<ROWS, COLS> operator-(const Matrix<ROWS, COLS>& B) const{
    Matrix<ROWS, COLS> result = *this;
    result -= B;
    return result;
  }

  Matrix<ROWS, COLS> operator*(float scalar) const{
    Matrix<ROWS, COLS> result = *this;
    result *= scalar;
    return result;
  }

  template<uint8_t OTHER_COLS>
  Matrix<ROWS, OTHER_COLS> operator*(const Matrix<COLS, OTHER_COLS>& B) const{
    Matrix<ROWS, OTHER_COLS> C = Matrix<ROWS, OTHER_COLS>::zeros();
    for(uint8_t i = 0; i < ROWS; ++i){
      for(uint8_t k = 0; k < COLS; ++k){
        float temp = m[i * COLS + k];
        for(uint8_t j = 0; j < OTHER_COLS; ++j){
          C.m[i * OTHER_COLS + j] += temp * B.m[k * OTHER_COLS + j];
        }
      }
    }
    return C;
  }

  Matrix<COLS, ROWS> transpose() const{
    Matrix<COLS, ROWS> T;
    for(uint8_t i = 0; i < ROWS; ++i){
      for(uint8_t j = 0; j < COLS; ++j){
        T.m[j * ROWS + i] = m[i * COLS + j];
      }
    }
    return T;
  }

  Matrix<3, 3> inverse3x3() const{
    static_assert(ROWS == 3 && COLS == 3, "inverse3x3 only valid for 3x3 matrices");
    Matrix<3, 3> inv;
    float det = m[0]*(m[4]*m[8] - m[5]*m[7]) -
                m[1]*(m[3]*m[8] - m[5]*m[6]) +
                m[2]*(m[3]*m[7] - m[4]*m[6]);

    if(fabs(det) < 1e-6f) return zeros();

    float inv_det = 1.0f / det;
    inv.m[0] =  (m[4]*m[8] - m[5]*m[7]) * inv_det;
    inv.m[1] = -(m[1]*m[8] - m[2]*m[7]) * inv_det;
    inv.m[2] =  (m[1]*m[5] - m[2]*m[4]) * inv_det;
    inv.m[3] = -(m[3]*m[8] - m[5]*m[6]) * inv_det;
    inv.m[4] =  (m[0]*m[8] - m[2]*m[6]) * inv_det;
    inv.m[5] = -(m[0]*m[5] - m[2]*m[3]) * inv_det;
    inv.m[6] =  (m[3]*m[7] - m[4]*m[6]) * inv_det;
    inv.m[7] = -(m[0]*m[7] - m[1]*m[6]) * inv_det;
    inv.m[8] =  (m[0]*m[4] - m[1]*m[3]) * inv_det;
    return inv;
  }

  // ==========================================
  // Vector Specific Functions (Nx1 vectors)
  // ==========================================

  // Magnitude of vector
  float norm() const{
    float sum = 0.0f;
    for(uint8_t i = 0; i < ROWS * COLS; ++i) sum += m[i] * m[i];
    return sqrtf(sum);
  }

  // In-place normalization (Crucial for Quaternions, Accel, Mag readings)
  void normalize(){
    float mag = norm();
    if(mag > 0.0f)
      for(uint8_t i = 0; i < ROWS * COLS; ++i) m[i] /= mag;
  }
};

typedef Matrix<7, 1> Vec7; // State Vector (x)
typedef Matrix<4, 1> Vec4; // Quaternions (q_pred, q_update)
typedef Matrix<3, 1> Vec3; // Sensor Readings, Innovation (z, h, y, omega)
