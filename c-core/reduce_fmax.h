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

CONST float reduce_fmax(int const size, float const array[restrict])
{
  float max = -INFINITY;
  int i = 0;
  float *x = (float *)ASSUME_ALIGNED(array);

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

#endif