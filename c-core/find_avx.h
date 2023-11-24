#ifndef FIND_AVX_H
#define FIND_AVX_H

#include "asint.h"
#include "compiler.h"
#include "find_generic.h"
#include <immintrin.h>

PURE int find2(float const needle, float const stack[restrict])
{
  return asint(needle) == asint(stack[0]) ? 0 : 1;
}

PURE int find3(float const needle, float const stack[restrict])
{
  if (asint(needle) == asint(stack[0])) return 0;
  if (asint(needle) == asint(stack[1])) return 1;
  return 2;
}

PURE int find5(float const needle, float const stack[restrict])
{
  __m128 x = _mm_set1_ps(needle);

  __m128 y = _mm_load_ps(stack);
  __m128i m = _mm_cmpeq_epi32(_mm_castps_si128(x), _mm_castps_si128(y));

  int mask = _mm_movemask_ps(_mm_castsi128_ps(m)) | 0x10;
  return __builtin_ctz(mask);
}

PURE int find10(float const needle, float const stack[restrict])
{
  __m256 x = _mm256_set1_ps(needle);
  __m256i xi = _mm256_castps_si256(x);

  __m256 y = _mm256_load_ps(stack);
  __m256i yi = _mm256_castps_si256(y);

  __m256i m = _mm256_cmpeq_epi32(xi, yi);

  if (!_mm256_testz_si256(m, m))
  {
    int mask = _mm256_movemask_ps(_mm256_castsi256_ps(m));
    return __builtin_ctz(mask);
  }
  if (stack[8] == needle) return 8;
  return 9;
}

PURE int find20(float const needle, float const stack[restrict])
{
  __m256 x = _mm256_set1_ps(needle);
  __m256i xi = _mm256_castps_si256(x);

  __m256 y1 = _mm256_load_ps(stack + 0);
  __m256 y2 = _mm256_load_ps(stack + 8);

  __m256i m1 = _mm256_cmpeq_epi32(xi, _mm256_castps_si256(y1));
  __m256i m2 = _mm256_cmpeq_epi32(xi, _mm256_castps_si256(y2));
  __m256i m = _mm256_or_si256(m1, m2);

  if (!_mm256_testz_si256(m, m))
  {
    int mask1 = _mm256_movemask_ps(_mm256_castsi256_ps(m1)) << 0;
    int mask2 = _mm256_movemask_ps(_mm256_castsi256_ps(m2)) << 8;
    return __builtin_ctz(mask1 + mask2);
  }

  __m128 z = _mm256_extractf128_ps(x, 0);
  __m128i zi = _mm_castps_si128(z);

  __m128 k = _mm_load_ps(stack + 16);
  __m128i ki = _mm_castps_si128(k);

  int mask = _mm_movemask_ps(_mm_castsi128_ps(_mm_cmpeq_epi32(zi, ki)));
  return __builtin_ctz(mask << 16);
}

#endif
