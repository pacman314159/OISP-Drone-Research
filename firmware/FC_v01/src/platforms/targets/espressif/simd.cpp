#include "platforms/hal/hal_simd.h"
#include <cmath>
#include "dsps_dotprod.h"
#include "dsps_add.h"
#include "dsps_sub.h"
#include "dsps_mulc.h"
#include "dsps_mat.h"

namespace hal_simd {

  // =========================================================================
  // 1. FLOATING-POINT (float) HARDWARE SIMD ROUTINES
  // =========================================================================

  float dot_product(const float* a, const float* b, size_t count){
    if(a == nullptr || b == nullptr || count == 0) return 0.0f;
    float result = 0.0f;
    dsps_dotprod_f32(a, b, &result, static_cast<int>(count));
    return result;
  }

  void add(const float* a, const float* b, float* out, size_t count){
    if(a == nullptr || b == nullptr || out == nullptr || count == 0) return;
    dsps_add_f32(a, b, out, static_cast<int>(count), 1, 1, 1);
  }

  void sub(const float* a, const float* b, float* out, size_t count){
    if(a == nullptr || b == nullptr || out == nullptr || count == 0) return;
    dsps_sub_f32(a, b, out, static_cast<int>(count), 1, 1, 1);
  }

  void scale(const float* in, float scalar, float* out, size_t count){
    if(in == nullptr || out == nullptr || count == 0) return;
    dsps_mulc_f32(in, out, static_cast<int>(count), scalar, 1, 1);
  }

  void mat_mul_f32(const float* A, const float* B, float* C, uint8_t m, uint8_t n, uint8_t k){
    if(A == nullptr || B == nullptr || C == nullptr || m == 0 || n == 0 || k == 0) return;
    dsps_mat_mul_f32(A, B, C, m, n, k);
  }

  bool inv3x3_f32(const float M[9], float Out[9]){
    if(M == nullptr || Out == nullptr) return false;

    const float* R0 = &M[0];
    const float* R1 = &M[3];
    const float* R2 = &M[6];

    float C0[3] = { R1[1]*R2[2] - R1[2]*R2[1], R1[2]*R2[0] - R1[0]*R2[2], R1[0]*R2[1] - R1[1]*R2[0] };
    float C1[3] = { R2[1]*R0[2] - R2[2]*R0[1], R2[2]*R0[0] - R2[0]*R0[2], R2[0]*R0[1] - R2[1]*R0[0] };
    float C2[3] = { R0[1]*R1[2] - R0[2]*R1[1], R0[2]*R1[0] - R0[0]*R1[2], R0[0]*R1[1] - R0[1]*R0[0] };

    float det = 0.0f;
    dsps_dotprod_f32(R0, C0, &det, 3);
    if(fabsf(det) < 1e-6f) return false;

    float inv_det = 1.0f / det;
    float C0_s[3], C1_s[3], C2_s[3];
    dsps_mulc_f32(C0, C0_s, 3, inv_det, 1, 1);
    dsps_mulc_f32(C1, C1_s, 3, inv_det, 1, 1);
    dsps_mulc_f32(C2, C2_s, 3, inv_det, 1, 1);

    Out[0] = C0_s[0]; Out[1] = C1_s[0]; Out[2] = C2_s[0];
    Out[3] = C0_s[1]; Out[4] = C1_s[1]; Out[5] = C2_s[1];
    Out[6] = C0_s[2]; Out[7] = C1_s[2]; Out[8] = C2_s[2];

    return true;
  }

  // =========================================================================
  // 2. 16-BIT SIGNED INTEGER (int16_t) ESP32-S3 PACKED HARDWARE SIMD ROUTINES
  // =========================================================================

  int16_t dot_product(const int16_t* a, const int16_t* b, size_t count){
    if(a == nullptr || b == nullptr || count == 0) return 0;
    int16_t result = 0;
    dsps_dotprod_s16(a, b, &result, static_cast<int>(count), 0);
    return result;
  }

  void add(const int16_t* a, const int16_t* b, int16_t* out, size_t count){
    if(a == nullptr || b == nullptr || out == nullptr || count == 0) return;
    dsps_add_s16(a, b, out, static_cast<int>(count), 1, 1, 1, 0);
  }

  void sub(const int16_t* a, const int16_t* b, int16_t* out, size_t count){
    if(a == nullptr || b == nullptr || out == nullptr || count == 0) return;
    dsps_sub_s16(a, b, out, static_cast<int>(count), 1, 1, 1, 0);
  }

  void scale(const int16_t* in, int16_t scalar, int16_t* out, size_t count){
    if(in == nullptr || out == nullptr || count == 0) return;
    dsps_mulc_s16(in, out, static_cast<int>(count), scalar, 1, 1, 0);
  }

  // =========================================================================
  // 3. 8-BIT SIGNED INTEGER (int8_t) ESP32-S3 PACKED HARDWARE SIMD ROUTINES
  // =========================================================================

  int8_t dot_product(const int8_t* a, const int8_t* b, size_t count){
    if(a == nullptr || b == nullptr || count == 0) return 0;
    int8_t result = 0;
    dsps_dotprod_s8(a, b, &result, static_cast<int>(count), 0);
    return result;
  }

  void add(const int8_t* a, const int8_t* b, int8_t* out, size_t count){
    if(a == nullptr || b == nullptr || out == nullptr || count == 0) return;
    dsps_add_s8(a, b, out, static_cast<int>(count), 1, 1, 1, 0);
  }

  void sub(const int8_t* a, const int8_t* b, int8_t* out, size_t count){
    if(a == nullptr || b == nullptr || out == nullptr || count == 0) return;
    dsps_sub_s8(a, b, out, static_cast<int>(count), 1, 1, 1, 0);
  }

  void scale(const int8_t* in, int8_t scalar, int8_t* out, size_t count){
    if(in == nullptr || out == nullptr || count == 0) return;
    dsps_mulc_s8(in, out, static_cast<int>(count), scalar, 1, 1, 0);
  }

} // namespace hal_simd
