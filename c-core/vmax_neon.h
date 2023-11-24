#ifndef VMAX_NEON_H
#define VMAX_NEON_H

#include "compiler.h"
#include "maximum.h"
#include <arm_neon.h>
#include <stdio.h>

PURE float vmax2(float const x[restrict]) { return maximum(x[0], x[1]); }

PURE float vmax3(float const x[restrict])
{
  float32x2_t t1 = vld1_f32(x);
  return maximum(x[2], vmaxv_f32(t1));
}

PURE float vmax5(float const x[restrict])
{
  float32x4_t t1 = vld1q_f32(x);
  return maximum(x[4], vmaxvq_f32(t1));
}

PURE float vmax10(float const x[restrict])
{
  float32x4_t t1 = vld1q_f32(x);
  float32x4_t t2 = vld1q_f32(x + 4);
  float32x2_t t3 = vld1_f32(x + 8);
  float32x4_t r = vmaxq_f32(vmaxq_f32(t1, t2), vcombine_f32(t3, t3));
  return vmaxvq_f32(r);
}

PURE float vmax20(float const x[restrict])
{
  float32x4_t t1 = vld1q_f32(x);
  float32x4_t t2 = vld1q_f32(x + 4);
  float32x4_t t3 = vld1q_f32(x + 8);
  float32x4_t t4 = vld1q_f32(x + 12);
  float32x4_t t5 = vld1q_f32(x + 16);
  float32x4_t r = vmaxq_f32(t1, t2);
  r = vmaxq_f32(t3, r);
  r = vmaxq_f32(t4, r);
  r = vmaxq_f32(t5, r);
  return vmaxvq_f32(r);
}

#endif
