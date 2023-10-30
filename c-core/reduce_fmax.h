#ifndef REDUCE_FMAX_H
#define REDUCE_FMAX_H

#include "compiler.h"
#include "float_maximum.h"
#include <float.h>
#include <math.h>

#if __ARM_NEON
#include <arm_neon.h>
#endif

#if __AVX__
#include <immintrin.h>
#endif

PURE float reduce_fmax(int const size, float const array[restrict])
{
  float max = -INFINITY;
  int i = 0;
  float *restrict x = ASSUME_ALIGNED(array);

#if __ARM_NEON
  float32x4_t r = vdupq_n_f32(-INFINITY);

  for (; i + 3 < size; i += 4)
    r = vmaxq_f32(r, vld1q_f32(&x[i]));
#endif
#if __AVX__
  __m256 r = _mm256_set1_ps(-INFINITY);
  for (; i + 7 < size; i += 8)
    r = _mm256_max_ps(r, _mm256_load_ps(&x[i]));
#endif

  for (; i < size; i++)
    max = float_maximum(max, x[i]);

#if __ARM_NEON
  if (size > 3) max = float_maximum(max, vmaxvq_f32(r));
#endif
#if __AVX__
  if (size > 7)
  {
    r = _mm256_max_ps(r, _mm256_permute_ps(r, _MM_SHUFFLE(2, 3, 0, 1)));
    r = _mm256_max_ps(r, _mm256_permute_ps(r, _MM_SHUFFLE(1, 0, 3, 2)));
    r = _mm256_max_ps(r, _mm256_permute2f128_ps(r, r, _MM_SHUFFLE(0, 0, 0, 1)));
    max = float_maximum(max, _mm_cvtss_f32(_mm256_castps256_ps128(r)));
  }
#endif

  return max;
}

#if __AVX__
PURE float reduce_m128(__m128 const x)
{
  __m128 t1 = _mm_movehl_ps(x, x);
  __m128 t2 = _mm_max_ps(x, t1);
  __m128 t3 = _mm_shuffle_ps(t2, t2, 1);
  __m128 t4 = _mm_max_ss(t2, t3);
  return _mm_cvtss_f32(t4);
}

PURE __m128 reduce_m256_onto_m128(__m256 const x)
{
  __m256 t1 = _mm256_permute_ps(x, _MM_SHUFFLE(2, 3, 0, 1));
  __m256 t2 = _mm256_max_ps(x, t1);
  __m256 t3 = _mm256_permute_ps(t2, _MM_SHUFFLE(1, 0, 3, 2));
  __m256 t4 = _mm256_max_ps(t2, t3);
  __m256 t5 = _mm256_permute2f128_ps(t4, t4, _MM_SHUFFLE(0, 0, 0, 1));
  __m256 t6 = _mm256_max_ps(t4, t5);
  return _mm256_castps256_ps128(t6);
}

PURE float reduce_m256(__m256 const x)
{
  return _mm_cvtss_f32(reduce_m256_onto_m128(x));
}

PURE float reduce_m256_m128(__m256 const x, __m128 const y)
{
  __m128 t1 = reduce_m256_onto_m128(x);
  __m128 t2 = _mm_max_ps(t1, y);
  return reduce_m128(t2);
}
#endif

PURE float reduce_fmax_size20(float const array[restrict])
{
  float const *restrict x = ASSUME_ALIGNED(array);
#if __AVX__
  __m256 t1 = _mm256_load_ps(x + 0);
  __m256 t2 = _mm256_load_ps(x + 8);
  __m128 t3 = _mm_load_ps(x + 16);
  __m256 t4 = _mm256_max_ps(t1, t2);
  return reduce_m256_m128(t4, t3);
#endif
#if __ARM_NEON
  return reduce_fmax(20, x);
#endif
}

PURE float reduce_fmax_size10(float const array[restrict])
{
  float const *restrict x = ASSUME_ALIGNED(array);
  return reduce_fmax(10, x);
}

PURE float reduce_fmax_size5(float const array[restrict])
{
  float const *restrict x = ASSUME_ALIGNED(array);
#if __AVX__
  __m128 t1 = _mm_load_ps(x);
  float t2 = reduce_m128(t1);
  return float_maximum(t2, x[4]);
#endif
#if __ARM_NEON
  return reduce_fmax(5, x);
#endif
}

PURE float reduce_fmax_size3(float const array[restrict])
{
  float const *restrict x = ASSUME_ALIGNED(array);
  return float_maximum(float_maximum(x[0], x[1]), x[2]);
}

#endif
