#pragma once

#include <Arduino.h>
#include <string.h>
#include <math.h>
#include <initializer_list>

template<uint8_t ROWS, uint8_t COLS>
class Matrix {
public:
    // 1D array flattening for guaranteed contiguous memory and cache speed
    // This perfectly fits into the ESP32's fast static RAM
    float m[ROWS * COLS];

    // Default constructor does NOTHING. Prevents double-writing 0s for speed.
    Matrix() = default;

    // Initializer list constructor (e.g., Matrix<7,1> x = {1, 0, 0, 0, 0, 0, 0};)
    Matrix(std::initializer_list<float> values) {
        uint8_t i = 0;
        for (float val : values) {
            if (i < ROWS * COLS) m[i++] = val;
        }
    }

    // Explicit zero initialization (Useful for H_a, H_m, Q, etc.)
    static Matrix<ROWS, COLS> zeros() {
        Matrix<ROWS, COLS> Z;
        memset(Z.m, 0, sizeof(Z.m)); // Hardware-accelerated memory set
        return Z;
    }

    // Explicit identity matrix (Useful for P, F, Ra, Rm)
    static Matrix<ROWS, COLS> identity() {
        static_assert(ROWS == COLS, "Identity only valid for square matrices");
        Matrix<ROWS, COLS> I = zeros();
        for(uint8_t i = 0; i < ROWS; i++) {
            I.m[i * COLS + i] = 1.0f;
        }
        return I;
    }

    // Easy access operator: M(row, col) - zero indexed!
    float& operator()(uint8_t row, uint8_t col) {
        return m[row * COLS + col];
    }
    
    const float& operator()(uint8_t row, uint8_t col) const {
        return m[row * COLS + col];
    }

    // ==========================================
    // In-Place Operators (Prevents Stack Copies)
    // ==========================================
    Matrix<ROWS, COLS>& operator+=(const Matrix<ROWS, COLS>& B) {
        for (uint8_t i = 0; i < ROWS * COLS; ++i) this->m[i] += B.m[i];
        return *this;
    }

    Matrix<ROWS, COLS>& operator-=(const Matrix<ROWS, COLS>& B) {
        for (uint8_t i = 0; i < ROWS * COLS; ++i) this->m[i] -= B.m[i];
        return *this;
    }

    Matrix<ROWS, COLS>& operator*=(float scalar) {
        for (uint8_t i = 0; i < ROWS * COLS; ++i) this->m[i] *= scalar;
        return *this;
    }

    // ==========================================
    // Standard Math Operators
    // ==========================================
    Matrix<ROWS, COLS> operator+(const Matrix<ROWS, COLS>& B) const {
        Matrix<ROWS, COLS> result = *this;
        result += B;
        return result;
    }

    Matrix<ROWS, COLS> operator-(const Matrix<ROWS, COLS>& B) const {
        Matrix<ROWS, COLS> result = *this;
        result -= B;
        return result;
    }

    Matrix<ROWS, COLS> operator*(float scalar) const {
        Matrix<ROWS, COLS> result = *this;
        result *= scalar;
        return result;
    }

    // Cache-Friendly Matrix Multiplication (M * N)
    template<uint8_t OTHER_COLS>
    Matrix<ROWS, OTHER_COLS> operator*(const Matrix<COLS, OTHER_COLS>& B) const {
        Matrix<ROWS, OTHER_COLS> C = Matrix<ROWS, OTHER_COLS>::zeros();
        // Loop order i-k-j is much faster for 1D flattened arrays on ESP32
        for (uint8_t i = 0; i < ROWS; ++i) {
            for (uint8_t k = 0; k < COLS; ++k) {
                float temp = m[i * COLS + k];
                for (uint8_t j = 0; j < OTHER_COLS; ++j) {
                    C.m[i * OTHER_COLS + j] += temp * B.m[k * OTHER_COLS + j];
                }
            }
        }
        return C;
    }

    // ==========================================
    // EKF Specific Functions
    // ==========================================
    
    // Transpose (Required for P * H^T calculations)
    Matrix<COLS, ROWS> transpose() const {
        Matrix<COLS, ROWS> T;
        for (uint8_t i = 0; i < ROWS; ++i) {
            for (uint8_t j = 0; j < COLS; ++j) {
                T.m[j * ROWS + i] = m[i * COLS + j];
            }
        }
        return T;
    }

    // Hardcoded 3x3 Algebraic Inversion 
    // Specifically for calculating the Innovation Covariance (S^-1)
    Matrix<3, 3> inverse3x3() const {
        static_assert(ROWS == 3 && COLS == 3, "inverse3x3 only valid for 3x3 matrices like S_a and S_m");
        Matrix<3, 3> inv;
        
        float det = m[0]*(m[4]*m[8] - m[5]*m[7]) -
                    m[1]*(m[3]*m[8] - m[5]*m[6]) +
                    m[2]*(m[3]*m[7] - m[4]*m[6]);
        
        // Prevent fatal divide-by-zero crashes mid-flight if matrix collapses
        if (fabs(det) < 1e-6f) return zeros(); 

        float invDet = 1.0f / det;
        inv.m[0] =  (m[4]*m[8] - m[5]*m[7]) * invDet;
        inv.m[1] = -(m[1]*m[8] - m[2]*m[7]) * invDet;
        inv.m[2] =  (m[1]*m[5] - m[2]*m[4]) * invDet;
        inv.m[3] = -(m[3]*m[8] - m[5]*m[6]) * invDet;
        inv.m[4] =  (m[0]*m[8] - m[2]*m[6]) * invDet;
        inv.m[5] = -(m[0]*m[5] - m[2]*m[3]) * invDet;
        inv.m[6] =  (m[3]*m[7] - m[4]*m[6]) * invDet;
        inv.m[7] = -(m[0]*m[7] - m[1]*m[6]) * invDet;
        inv.m[8] =  (m[0]*m[4] - m[1]*m[3]) * invDet;
        return inv;
    }

    // ==========================================
    // Vector Specific Functions (Only use on Nx1 vectors)
    // ==========================================
    
    // Length / Magnitude of the vector
    float norm() const {
        float sum = 0.0f;
        for (uint8_t i = 0; i < ROWS * COLS; ++i) sum += m[i] * m[i];
        return sqrtf(sum);
    }

    // In-place normalization (Crucial for Quaternions, Accel, Mag readings)
    void normalize() {
        float mag = norm();
        if (mag > 0.0f) {
            for (uint8_t i = 0; i < ROWS * COLS; ++i) m[i] /= mag;
        }
    }

    // Debugging print
    void print(const char* name = "Matrix") const {
        Serial.println(name);
        for (uint8_t i = 0; i < ROWS; ++i) {
            for (uint8_t j = 0; j < COLS; ++j) {
                Serial.print(m[i * COLS + j], 6);
                Serial.print("\t");
            }
            Serial.println();
        }
        Serial.println();
    }
};

// ==========================================
// Custom Typedefs mapping exactly to your table
// ==========================================
typedef Matrix<7, 1> Vector7;   // State Vector (x)
typedef Matrix<4, 1> Vector4;   // Quaternions (q_pred, q_update)
typedef Matrix<3, 1> Vector3;   // Sensor Readings, Innovation (z, h, y, omega)