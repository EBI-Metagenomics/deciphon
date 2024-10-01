#ifndef INTRINSICS_H
#define INTRINSICS_H

#include <inttypes.h>
#include <math.h>

#ifndef NDEBUG
#include <stdio.h>
#endif

#if __ARM_NEON
  #include <arm_neon.h>
  typedef float32x4_t packf;
  typedef uint32x4_t  packu;
  #define ALIGNMENT 16
  #define NUM_LANES 4
#elif __AVX512F__
  #include <immintrin.h>
  typedef __m512  packf;
  typedef __m512i packu;
  #define ALIGNMENT 64
  #define NUM_LANES 16
#elif __AVX__
  #include <immintrin.h>
  typedef __m256  packf;
  typedef __m256i packu;
  #define ALIGNMENT 32
  #define NUM_LANES 8
#else
  #error "We require either AVX or NEON feature."
#endif

typedef uint32_t u32;
typedef float    f32;

#if __ARM_NEON
#define add(a, b)          vaddq_f32(a, b)
#define and(a, b)          vandq_u32(a, b)
#define blendf(m, a, b)    vbslq_f32(m, a, b)
#define blendu(m, a, b)    vbslq_u32(m, a, b)
#define castf(x)           (packf)(x)
#define castu(x)           (packu)(x)
#define dupf(x)            vdupq_n_f32(x)
#define dupu(x)            vdupq_n_u32(x)
#define eq(a, b)           vceqq_f32(a, b)
#define init4u(a, b, c, d) ((packu){a, b, c, d})
#define init4f(a, b, c, d) ((packf){a, b, c, d})
#define loadf(mem)         vld1q_f32(mem)
#define loadu(mem)         vld1q_u32(mem)
#define maxu(a, b)         vmaxq_u32(a, b)
#define min(a, b)          vminq_f32(a, b)
#define or(a, b)           vorrq_u32(a, b)
#define storef(mem, x)     vst1q_f32(mem, x)
#define storeu(mem, x)     vst1q_u32(mem, x)
#elif __AVX512F__
#define add(a, b)                                               _mm512_add_ps(a, b)
#define and(a, b)                                               _mm512_and_si512(a, b)
#define blendf(m, a, b)                                         _mm512_mask_blend_ps(m, b, a)
#define blendu(m, a, b)                                         _mm512_mask_blend_epi32(m, b, a)
#define castf(x)                                                _mm512_castsi512_ps(x)
#define castu(x)                                                _mm512_castps_si512(x)
#define dupf(x)                                                 _mm512_set1_ps(x)
#define dupu(x)                                                 _mm512_set1_epi32(x)
#define eq(a, b)                                                _mm512_cmp_ps_mask(a, b, _CMP_EQ_OQ)
#define init16u(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) _mm512_setr_epi32(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p)
#define init16f(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) _mm512_setr_ps(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p)
#define loadf(mem)                                              _mm512_loadu_ps(mem)
#define loadu(mem)                                              _mm512_loadu_si512(mem)
#define maxu(a, b)                                              _mm512_max_epi32(a, b)
#define min(a, b)                                               _mm512_min_ps(a, b)
#define or(a, b)                                                _mm512_or_si512(a, b)
#define storef(mem, x)                                          _mm512_storeu_ps(mem, x)
#define storeu(mem, x)                                          _mm512_storeu_si512((packu *)mem, x)
#elif __AVX__
#define add(a, b)                      _mm256_add_ps(a, b)
#define and(a, b)                      _mm256_and_si256(a, b)
#define blendf(m, a, b)                _mm256_blendv_ps(b, a, castf(m))
#define blendu(m, a, b)                _mm256_blendv_epi8(b, a, m)
#define castf(x)                       _mm256_castsi256_ps(x)
#define castu(x)                       _mm256_castps_si256(x)
#define dupf(x)                        _mm256_set1_ps(x)
#define dupu(x)                        _mm256_set1_epi32(x)
#define eq(a, b)                       castu(_mm256_cmp_ps(a, b, _CMP_EQ_OQ))
#define init8u(a, b, c, d, e, f, g, h) _mm256_setr_epi32(a, b, c, d, e, f, g, h)
#define init8f(a, b, c, d, e, f, g, h) _mm256_setr_ps(a, b, c, d, e, f, g, h)
#define loadf(mem)                     _mm256_loadu_ps(mem)
#define loadu(mem)                     _mm256_loadu_si256(mem)
#define maxu(a, b)                     _mm256_max_epi32(a, b)
#define min(a, b)                      _mm256_min_ps(a, b)
#define or(a, b)                       _mm256_or_si256(a, b)
#define storef(mem, x)                 _mm256_storeu_ps(mem, x)
#define storeu(mem, x)                 _mm256_storeu_si256((packu *)mem, x)
#endif

static inline __attribute__((always_inline)) packf shift(packf x)
{
#if __ARM_NEON
  return vextq_f32(dupf(INFINITY), x, NUM_LANES - 1);
#elif __AVX512F__
  x = _mm512_permutexvar_ps(init16u(15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14), x);
  return _mm512_mask_blend_ps(0x0001, x, dupf(INFINITY));
#elif __AVX__
  x = _mm256_permutevar8x32_ps(x, init8u(7, 0, 1, 2, 3, 4, 5, 6));
  return _mm256_blend_ps(x, dupf(INFINITY), _MM_SHUFFLE(0, 0, 0, 1));
#endif
}

static inline __attribute__((always_inline)) u32 hmaxu(packu x)
{
#if __ARM_NEON
  return vmaxvq_u32(x);
#elif __AVX512F__
  x = maxu(x, castu(_mm512_permute_ps(castf(x), _MM_SHUFFLE(2, 3, 0, 1))));
  x = maxu(x, castu(_mm512_permute_ps(castf(x), _MM_SHUFFLE(1, 0, 3, 2))));
  x = maxu(x, castu(_mm512_shuffle_f32x4(castf(x), castf(x), _MM_SHUFFLE(2, 3, 0, 1))));
  x = maxu(x, castu(_mm512_shuffle_f32x4(castf(x), castf(x), _MM_SHUFFLE(1, 0, 3, 2))));
  return _mm_cvtsi128_si32(_mm512_castsi512_si128(x));
#elif __AVX__
  x = maxu(x, castu(_mm256_permute_ps(castf(x), _MM_SHUFFLE(2, 3, 0, 1))));
  x = maxu(x, castu(_mm256_permute_ps(castf(x), _MM_SHUFFLE(1, 0, 3, 2))));
  x = maxu(x, _mm256_permute2f128_si256(x, x, _MM_SHUFFLE(0, 0, 0, 1)));
  return _mm_cvtsi128_si32(_mm256_castsi256_si128(x));
#endif
}

static inline __attribute__((always_inline)) f32 hmin(packf x)
{
#if __ARM_NEON
  return vminvq_f32(x);
#elif __AVX512F__
  x = min(x, _mm512_permute_ps(x, _MM_SHUFFLE(2, 3, 0, 1)));
  x = min(x, _mm512_permute_ps(x, _MM_SHUFFLE(1, 0, 3, 2)));
  x = min(x, _mm512_shuffle_f32x4(x, x, _MM_SHUFFLE(2, 3, 0, 1)));
  x = min(x, _mm512_shuffle_f32x4(x, x, _MM_SHUFFLE(1, 0, 3, 2)));
  return _mm_cvtss_f32(_mm512_castps512_ps128(x));
#elif __AVX__
  x = min(x, _mm256_permute_ps(x, _MM_SHUFFLE(2, 3, 0, 1)));
  x = min(x, _mm256_permute_ps(x, _MM_SHUFFLE(1, 0, 3, 2)));
  x = min(x, _mm256_permute2f128_ps(x, x, _MM_SHUFFLE(0, 0, 0, 1)));
  return _mm_cvtss_f32(_mm256_castps256_ps128(x));
#endif
}

static inline __attribute__((always_inline)) void min_idx(packf *minval, packf val, packu *minidx, packu idx)
{
  packf x = min(*minval, val);
  *minidx = blendu(eq(x, *minval), *minidx, idx);
  *minval = x;
}

static inline __attribute__((always_inline)) void hmin_idx(f32 *minval, packf val, u32 *minidx, packu idx)
{
  f32 x = hmin(val);
#if __AVX512F__
  *minidx = hmaxu(blendu(eq(val, dupf(x)), idx, dupu(0)));
#else
  *minidx = hmaxu(and(eq(val, dupf(x)), idx));
#endif
  *minval = x;
}

static inline __attribute__((always_inline)) int all_leq(packf a, packf b)
{
#if __ARM_NEON
  packu m = vmvnq_u32(eq(min(a, b), a));
  uint32x2_t r = vshrn_n_u64(vreinterpretq_u64_u32(m), 16);
  return !vget_lane_u64(vreinterpret_u64_u32(r), 0);
#elif __AVX512F__
  return 0xFFFF == _mm512_cmp_ps_mask(a, b, _CMP_LE_OS);
#elif __AVX__
  return 0xFF == _mm256_movemask_ps(_mm256_cmp_ps(a, b, _CMP_LE_OS));
#endif
}

static inline __attribute__((always_inline)) void setf(packf *x, f32 v, int e)
{
  packf broad = dupf(v);
#if __AVX512F__
  __mmask16 mask = 1 << e;
#else
  int32_t m[2 * NUM_LANES] = {0};
  m[NUM_LANES] = -1;
  packu mask = loadu((void const *)(m + NUM_LANES - (e & (NUM_LANES - 1))));
#endif
  *x = blendf(mask, broad, *x);
}

static inline __attribute__((always_inline)) void setu(packu *x, u32 v, int e)
{
  packu broad = dupu(v);
#if __AVX512F__
  __mmask16 mask = 1 << e;
#else
  int32_t m[2 * NUM_LANES] = {0};
  m[NUM_LANES] = -1;
  packu mask = loadu((void const *)(m + NUM_LANES - (e & (NUM_LANES - 1))));
#endif
  *x = blendu(mask, broad, *x);
}

static inline __attribute__((always_inline)) f32 getf(packf x, int e)
{
  f32 arr[NUM_LANES];
  storef(arr, x);
  return arr[e & (NUM_LANES - 1)];
}

static inline __attribute__((always_inline)) u32 getu(packu x, int e)
{
  u32 arr[NUM_LANES];
  storeu(arr, x);
  return arr[e & (NUM_LANES - 1)];
}

__attribute__((unused)) static void echof(packf x)
{
  for (int i = 0; i < NUM_LANES; ++i)
    printf("%6.4f ", getf(x, i));
  printf("\n");
}

__attribute__((unused)) static void echou(packu x)
{
  for (int i = 0; i < NUM_LANES; ++i)
    printf("%#010x ", getu(x, i));
  printf("\n");
}

#endif
