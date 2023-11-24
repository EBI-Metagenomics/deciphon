#ifndef VMAX_AVX_H
#define VMAX_AVX_H

#include "compiler.h"
#include "maximum.h"
#include <immintrin.h>

CONST float m128max(__m128 const x)
{
  __m128 t1 = _mm_movehl_ps(x, x);
  __m128 t2 = _mm_max_ps(x, t1);
  __m128 t3 = _mm_shuffle_ps(t2, t2, 1);
  __m128 t4 = _mm_max_ss(t2, t3);
  return _mm_cvtss_f32(t4);
}

CONST float m256max(__m256 const x)
{
  __m256 r = x;
  r = _mm256_max_ps(r, _mm256_permute_ps(r, _MM_SHUFFLE(2, 3, 0, 1)));
  r = _mm256_max_ps(r, _mm256_permute_ps(r, _MM_SHUFFLE(1, 0, 3, 2)));
  r = _mm256_max_ps(r, _mm256_permute2f128_ps(r, r, _MM_SHUFFLE(0, 0, 0, 1)));
  return _mm_cvtss_f32(_mm256_castps256_ps128(r));
}

PURE float vmax2(float const x[restrict]) { return maximum(x[0], x[1]); }

PURE float vmax3(float const x[restrict])
{
  return maximum(maximum(x[0], x[1]), x[2]);
}

PURE float vmax4(float const x[restrict])
{
  __m128 t1 = _mm_load_ps(x);
  __m128 t2 = _mm_movehl_ps(t1, t1);
  __m128 t3 = _mm_max_ps(t1, t2);
  __m128 t4 = _mm_shuffle_ps(t3, t3, 1);
  __m128 t5 = _mm_max_ss(t3, t4);
  return _mm_cvtss_f32(t5);
}

PURE float vmax5(float const x[restrict]) { return maximum(vmax4(x), x[4]); }

PURE float vmax10(float const x[restrict])
{
  float r = m256max(_mm256_load_ps(x));
  return maximum(maximum(x[8], x[9]), r);
}

PURE float vmax20(float const x[restrict])
{
  __m256 r1 = _mm256_load_ps(x + 0);
  __m256 r2 = _mm256_load_ps(x + 8);
  __m256 r3 = _mm256_set_m128(_mm_load_ps(x + 16), _mm_load_ps(x + 16));
  return m256max(_mm256_max_ps(_mm256_max_ps(r1, r2), r3));
}

#endif
