#ifndef FIND_NEON_H
#define FIND_NEON_H

#if __ARM_NEON
#include "compiler.h"
#include <arm_neon.h>

PURE uint64_t neon_mask64(int32x2_t const needle, float const stack[restrict])
{
  uint32x2_t m = vceq_s32(needle, vreinterpret_s32_f32(vld1_f32(stack)));
  return vget_lane_u64(vreinterpret_u64_u32(m), 0);
}

PURE uint64_t neon_mask128(int32x4_t const needle, float const stack[restrict])
{
  uint32x4_t m = vceqq_s32(needle, vreinterpretq_s32_f32(vld1q_f32(stack)));
  uint32x2_t r = vshrn_n_u64(vreinterpretq_u64_u32(m), 16);
  return vget_lane_u64(vreinterpret_u64_u32(r), 0);
}

PURE int find2(float const needle, float const stack[restrict])
{
  int32x2_t x = vreinterpret_s32_f32(vdup_n_f32(needle));

  uint64_t f = neon_mask64(x, stack);
  return __builtin_ctzll(f) >> 5;
}

PURE int find3(float const needle, float const stack[restrict])
{
  int32x2_t x = vreinterpret_s32_f32(vdup_n_f32(needle));

  uint64_t f = neon_mask64(x, stack);
  return f ? __builtin_ctzll(f) >> 5 : 2;
}

PURE int find5(float const needle, float const stack[restrict])
{
  int32x4_t x = vreinterpretq_s32_f32(vdupq_n_f32(needle));

  uint64_t f = neon_mask128(x, stack + 0);
  return f ? __builtin_ctzll(f) >> 4 : 4;
}

PURE int find10(float const needle, float const stack[restrict])
{
  int32x4_t x = vreinterpretq_s32_f32(vdupq_n_f32(needle));

  uint64_t f = neon_mask128(x, stack + 0);
  if (f) return (__builtin_ctzll(f) >> 4) + 0;

  f = neon_mask128(x, stack + 4);
  if (f) return (__builtin_ctzll(f) >> 4) + 4;

  return needle == stack[8] ? 8 : 9;
}

PURE int find20(float const needle, float const stack[restrict])
{
  int32x4_t x = vreinterpretq_s32_f32(vdupq_n_f32(needle));

  uint64_t f = neon_mask128(x, stack + 0);
  if (f) return (__builtin_ctzll(f) >> 4) + 0;

  f = neon_mask128(x, stack + 4);
  if (f) return (__builtin_ctzll(f) >> 4) + 4;

  f = neon_mask128(x, stack + 8);
  if (f) return (__builtin_ctzll(f) >> 4) + 8;

  f = neon_mask128(x, stack + 12);
  if (f) return (__builtin_ctzll(f) >> 4) + 12;

  f = neon_mask128(x, stack + 16);
  return (__builtin_ctzll(f) >> 4) + 16;
}
#endif

#endif
