#pragma once

#include <cstdint>
#include <cstring>
#include <cmath>
#include <initializer_list>
#include <type_traits>
#include "config.h"
#include "platforms/hal/hal_simd.h"

template<typename T = float, uint8_t ROWS = 3, uint8_t COLS = 1>
class Matrix {
public:
  union {
    T m[ROWS * COLS];
    struct { T x, y, z; };
  };

  Matrix() : m{0}{}

  Matrix(std::initializer_list<T> values){
    uint8_t i = 0;
    for(T val : values)
      if(i < ROWS * COLS) m[i++] = val;
  }

  static Matrix<T, ROWS, COLS> zeros(){
    Matrix<T, ROWS, COLS> Z;
    memset(Z.m, 0, sizeof(Z.m));
    return Z;
  }

  static Matrix<T, ROWS, COLS> identity(){
    static_assert(ROWS == COLS, "Identity only valid for square matrices");
    Matrix<T, ROWS, COLS> I = zeros();
    for(uint8_t i = 0; i < ROWS; i++)
      I.m[i * COLS + i] = static_cast<T>(1);
    return I;
  }

  T& operator()(uint8_t row, uint8_t col){
    return m[row * COLS + col];
  }

  const T& operator()(uint8_t row, uint8_t col) const {
    return m[row * COLS + col];
  }

  T& operator[](size_t index){
    return m[index];
  }

  const T& operator[](size_t index) const {
    return m[index];
  }

  Matrix<T, COLS, ROWS> transpose() const {
    Matrix<T, COLS, ROWS> T_mat;
    for(uint8_t i = 0; i < ROWS; ++i)
      for(uint8_t j = 0; j < COLS; ++j)
        T_mat.m[j * ROWS + i] = m[i * COLS + j];
    return T_mat;
  }

  // =========================================================================
  // MATHEMATICAL OPERATIONS (Conventional Block vs SIMD Accelerated Block)
  // =========================================================================
#if (!ENABLE_SIMD_ACCELERATION)

  Matrix<T, ROWS, COLS>& operator+=(const Matrix<T, ROWS, COLS>& B){
    for(uint8_t i = 0; i < ROWS * COLS; ++i) this->m[i] += B.m[i];
    return *this;
  }

  Matrix<T, ROWS, COLS>& operator-=(const Matrix<T, ROWS, COLS>& B){
    for(uint8_t i = 0; i < ROWS * COLS; ++i) this->m[i] -= B.m[i];
    return *this;
  }

  Matrix<T, ROWS, COLS>& operator*=(T scalar){
    for(uint8_t i = 0; i < ROWS * COLS; ++i) this->m[i] *= scalar;
    return *this;
  }

  Matrix<T, ROWS, COLS> operator+(const Matrix<T, ROWS, COLS>& B) const {
    Matrix<T, ROWS, COLS> res = *this;
    res += B;
    return res;
  }

  Matrix<T, ROWS, COLS> operator-(const Matrix<T, ROWS, COLS>& B) const {
    Matrix<T, ROWS, COLS> res = *this;
    res -= B;
    return res;
  }

  Matrix<T, ROWS, COLS> operator*(T scalar) const {
    Matrix<T, ROWS, COLS> res = *this;
    res *= scalar;
    return res;
  }

  template<uint8_t OTHER_COLS>
  Matrix<T, ROWS, OTHER_COLS> operator*(const Matrix<T, COLS, OTHER_COLS>& B) const {
    Matrix<T, ROWS, OTHER_COLS> C;
    memset(C.m, 0, sizeof(C.m));
    for(uint8_t i = 0; i < ROWS; ++i)
      for(uint8_t k = 0; k < COLS; ++k){
        T temp = m[i * COLS + k];
        for(uint8_t j = 0; j < OTHER_COLS; ++j)
          C.m[i * OTHER_COLS + j] += temp * B.m[k * OTHER_COLS + j];
      }
    return C;
  }

  T norm() const {
    T sum = 0;
    for(uint8_t i = 0; i < ROWS * COLS; ++i) sum += m[i] * m[i];
    return static_cast<T>(sqrt(sum));
  }

  void normalize(){
    T mag = norm();
    if(mag > static_cast<T>(0))
      *this *= (static_cast<T>(1) / mag);
  }

  Matrix<T, 3, 3> inverse3x3() const {
    static_assert(ROWS == 3 && COLS == 3, "inverse3x3 only valid for 3x3 matrices");
    Matrix<T, 3, 3> inv;
    T det = m[0]*(m[4]*m[8] - m[5]*m[7]) -
            m[1]*(m[3]*m[8] - m[5]*m[6]) +
            m[2]*(m[3]*m[7] - m[4]*m[6]);

    if(fabs(static_cast<float>(det)) < 1e-6f) return zeros();

    T inv_det = static_cast<T>(1) / det;
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

#else // ENABLE_SIMD_ACCELERATION (Hardware SIMD Vector Math Acceleration)

  Matrix<T, ROWS, COLS>& operator+=(const Matrix<T, ROWS, COLS>& B){
    if constexpr (std::is_same_v<T, float> || std::is_same_v<T, int16_t> || std::is_same_v<T, int8_t>)
      hal_simd::add(this->m, B.m, this->m, ROWS * COLS);
    else
      for(uint8_t i = 0; i < ROWS * COLS; ++i) this->m[i] += B.m[i];
    return *this;
  }

  Matrix<T, ROWS, COLS>& operator-=(const Matrix<T, ROWS, COLS>& B){
    if constexpr (std::is_same_v<T, float> || std::is_same_v<T, int16_t> || std::is_same_v<T, int8_t>)
      hal_simd::sub(this->m, B.m, this->m, ROWS * COLS);
    else
      for(uint8_t i = 0; i < ROWS * COLS; ++i) this->m[i] -= B.m[i];
    return *this;
  }

  Matrix<T, ROWS, COLS>& operator*=(T scalar){
    if constexpr (std::is_same_v<T, float> || std::is_same_v<T, int16_t> || std::is_same_v<T, int8_t>)
      hal_simd::scale(this->m, scalar, this->m, ROWS * COLS);
    else
      for(uint8_t i = 0; i < ROWS * COLS; ++i) this->m[i] *= scalar;
    return *this;
  }

  Matrix<T, ROWS, COLS> operator+(const Matrix<T, ROWS, COLS>& B) const {
    Matrix<T, ROWS, COLS> res = *this;
    res += B;
    return res;
  }

  Matrix<T, ROWS, COLS> operator-(const Matrix<T, ROWS, COLS>& B) const {
    Matrix<T, ROWS, COLS> res = *this;
    res -= B;
    return res;
  }

  Matrix<T, ROWS, COLS> operator*(T scalar) const {
    Matrix<T, ROWS, COLS> res = *this;
    res *= scalar;
    return res;
  }

  template<uint8_t OTHER_COLS>
  Matrix<T, ROWS, OTHER_COLS> operator*(const Matrix<T, COLS, OTHER_COLS>& B) const {
    Matrix<T, ROWS, OTHER_COLS> C;
    if constexpr (std::is_same_v<T, float>)
      hal_simd::mat_mul_f32(this->m, B.m, C.m, ROWS, COLS, OTHER_COLS);
    else{
      memset(C.m, 0, sizeof(C.m));
      for(uint8_t i = 0; i < ROWS; ++i)
        for(uint8_t k = 0; k < COLS; ++k){
          T temp = m[i * COLS + k];
          for(uint8_t j = 0; j < OTHER_COLS; ++j)
            C.m[i * OTHER_COLS + j] += temp * B.m[k * OTHER_COLS + j];
        }
    }
    return C;
  }

  T norm() const {
    if constexpr (std::is_same_v<T, float> || std::is_same_v<T, int16_t> || std::is_same_v<T, int8_t>)
      return static_cast<T>(sqrt(hal_simd::dot_product(this->m, this->m, ROWS * COLS)));
    else{
      T sum = 0;
      for(uint8_t i = 0; i < ROWS * COLS; ++i) sum += m[i] * m[i];
      return static_cast<T>(sqrt(sum));
    }
  }

  void normalize(){
    T mag = norm();
    if(mag > static_cast<T>(0))
      *this *= (static_cast<T>(1) / mag);
  }

  Matrix<T, 3, 3> inverse3x3() const {
    static_assert(ROWS == 3 && COLS == 3, "inverse3x3 only valid for 3x3 matrices");
    Matrix<T, 3, 3> inv;
    if constexpr (std::is_same_v<T, float>){
      if(hal_simd::inv3x3_f32(this->m, inv.m)) return inv;
      return zeros();
    }else{
      T det = m[0]*(m[4]*m[8] - m[5]*m[7]) -
              m[1]*(m[3]*m[8] - m[5]*m[6]) +
              m[2]*(m[3]*m[7] - m[4]*m[6]);

      if(fabs(static_cast<float>(det)) < 1e-6f) return zeros();

      T inv_det = static_cast<T>(1) / det;
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
  }

#endif
};

typedef Matrix<float, 7, 1> Vec7; // State Vector (x)
typedef Matrix<float, 4, 1> Vec4; // Quaternions (q_pred, q_update)

// Vec3 inherits Matrix<T, 3, 1>
template<typename T = float>
struct Vec3 : public Matrix<T, 3, 1> {
  using Matrix<T, 3, 1>::Matrix;

  Vec3() : Matrix<T, 3, 1>{0, 0, 0}{}
  Vec3(T x_, T y_, T z_) : Matrix<T, 3, 1>{x_, y_, z_}{}
  Vec3(const Matrix<T, 3, 1>& mat) : Matrix<T, 3, 1>(mat){}
};
