#include "viterbi.h"
#include "trellis.h"
#include "xlimits.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#if __ARM_NEON
  #include <arm_neon.h>
  typedef float32x4_t packf;
  typedef uint32x4_t  packu;
  #define ALIGNMENT 16
  #define NUM_LANES 4
#elif __AVX__
  #include <immintrin.h>
  typedef __m256  packf;
  typedef __m256i packu;
  #define ALIGNMENT 32
  #define NUM_LANES 8
#else
  #error "We require either AVX or NEON feature."
#endif

#define INLINE static inline __attribute__((always_inline))

typedef uint32_t u32;
typedef float    f32;

#define STEP_NAME_OFFSET 28
#define STEP_LANE_OFFSET 24
#define STEP_DATA_OFFSET 0

#define STEP_NAME_MASK 0xF0000000
#define STEP_LANE_MASK 0x0F000000
#define STEP_DATA_MASK 0x00FFFFFF

#define STEP_SN (0x1 << STEP_NAME_OFFSET)
#define STEP_NN (0x2 << STEP_NAME_OFFSET)

#define STEP_SB (0x1 << STEP_NAME_OFFSET)
#define STEP_NB (0x2 << STEP_NAME_OFFSET)
#define STEP_EB (0x4 << STEP_NAME_OFFSET)
#define STEP_JB (0x8 << STEP_NAME_OFFSET)

#define STEP_EJ (0x1 << STEP_NAME_OFFSET)
#define STEP_JJ (0x2 << STEP_NAME_OFFSET)

#define STEP_EC (0x1 << STEP_NAME_OFFSET)
#define STEP_CC (0x2 << STEP_NAME_OFFSET)

#define STEP_ET (0x1 << STEP_NAME_OFFSET)
#define STEP_CT (0x2 << STEP_NAME_OFFSET)

#define STEP_BM (0x1 << STEP_NAME_OFFSET)
#define STEP_MM (0x2 << STEP_NAME_OFFSET)
#define STEP_IM (0x4 << STEP_NAME_OFFSET)
#define STEP_DM (0x8 << STEP_NAME_OFFSET)

#define STEP_MI (0x1 << STEP_NAME_OFFSET)
#define STEP_II (0x2 << STEP_NAME_OFFSET)

#define STEP_MD (0x1 << STEP_NAME_OFFSET)
#define STEP_DD (0x2 << STEP_NAME_OFFSET)

#define STEP_ME (0x1 << STEP_NAME_OFFSET)
#define STEP_DE (0x2 << STEP_NAME_OFFSET)

INLINE u32 step(u32 n, int x)
{
  return (n & STEP_NAME_MASK) | ((STEP_LANE_MASK | STEP_DATA_MASK) & (u32)x);
}

INLINE int step_lane(u32 x)
{
  return (int)(x & STEP_LANE_MASK) >> STEP_LANE_OFFSET;
}

INLINE int step_data(u32 x)
{
  return (int)(STEP_DATA_MASK & x) >> STEP_DATA_OFFSET;
}

struct emission
{
  f32 null[DCP_ABC_TABLE_SIZE];
  packf background[DCP_ABC_TABLE_SIZE];
  packf *match;
};

struct extr_trans
{
  f32 RR;

  f32 SN;
  f32 NN;

  f32 SB;
  f32 NB;
  f32 EB;
  f32 JB;

  f32 EJ;
  f32 JJ;

  f32 EC;
  f32 CC;

  f32 ET;
  f32 CT;
};

struct core_trans
{
  packf BM;
  packf MM;
  packf MI;
  packf MD;
  packf IM;
  packf II;
  packf DM;
  packf DD;
};

struct extr_state
{
  // Prefix
  f32 S;
  f32 N;
  f32 B;

  // Infix
  f32 J;

  // Suffix
  f32 E;
  f32 C;
  f32 T;
};

struct core_state
{
  packf M;
  packf D;
  packf I;
};

struct prev_extr_state
{
  u32 S;
  u32 N;
  u32 B;

  u32 J;

  u32 C;
  u32 E;
  u32 T;
};

struct prev_core_state
{
  packu M;
  packu D;
  packu I;
};

struct viterbi
{
  int K;
  int Q;
  int maxQ;
  struct extr_state extr_state[DCP_PAST_SIZE];
  struct core_state *core_state;

  struct emission emission;

  struct extr_trans extr_trans;
  struct core_trans *core_trans;

  struct prev_extr_state prev_extr_state;
  struct prev_core_state *prev_core_state;
  struct trellis trellis;
};

#if __ARM_NEON
#define add(a, b)         vaddq_f32(a, b)
#define and(a, b)         vandq_u32(a, b)
#define blendf(m, a, b)   vbslq_f32(m, a, b)
#define blendu(m, a, b)   vbslq_u32(m, a, b)
#define castf(x)          (packf)(x)
#define castu(x)          (packu)(x)
#define dupf(x)           vdupq_n_f32(x)
#define dupu(x)           vdupq_n_u32(x)
#define eq(a, b)          vceqq_f32(a, b)
#define initu(a, b, c, d) ((packu){a, b, c, d})
#define initf(a, b, c, d) ((packf){a, b, c, d})
#define loadf(mem)        vld1q_f32(mem)
#define loadu(mem)        vld1q_u32(mem)
#define maxu(a, b)        vmaxq_u32(a, b)
#define min(a, b)         vminq_f32(a, b)
#define or(a, b)          vorrq_u32(a, b)
#define storef(mem, x)    vst1q_f32(mem, x)
#define storeu(mem, x)    vst1q_u32(mem, x)
#endif

#if __AVX__
#define add(a, b)                     _mm256_add_ps(a, b)
#define and(a, b)                     _mm256_and_si256(a, b)
#define blendf(m, a, b)               _mm256_blendv_ps(b, a, m)
#define blendu(m, a, b)               _mm256_blendv_epi8(b, a, m)
#define castf(x)                      _mm256_castsi256_ps(x)
#define castu(x)                      _mm256_castps_si256(x)
#define dupf(x)                       _mm256_set1_ps(x)
#define dupu(x)                       _mm256_set1_epi32(x)
#define eq(a, b)                      castu(_mm256_cmp_ps(a, b, _CMP_EQ_OQ))
#define initu(a, b, c, d, e, f, g, h) _mm256_setr_epi32(a, b, c, d, e, f, g, h)
#define initf(a, b, c, d, e, f, g, h) _mm256_setr_ps(a, b, c, d, e, f, g, h)
#define loadf(mem)                    _mm256_loadu_ps(mem)
#define loadu(mem)                    _mm256_loadu_si256(mem)
#define maxu(a, b)                    _mm256_max_epi32(a, b)
#define min(a, b)                     _mm256_min_ps(a, b)
#define or(a, b)                      _mm256_or_si256(a, b)
#define storef(mem, x)                _mm256_storeu_ps(mem, x)
#define storeu(mem, x)                _mm256_storeu_si256((packu *)mem, x)
#endif

INLINE packf shift(packf x)
{
#if __ARM_NEON
  return vextq_f32(dupf(INFINITY), x, NUM_LANES - 1);
#endif
#if __AVX__
  x = _mm256_permutevar8x32_ps(x, initu(7, 0, 1, 2, 3, 4, 5, 6));
  return _mm256_blend_ps(x, dupf(INFINITY), _MM_SHUFFLE(0, 0, 0, 1));
#endif
}

INLINE u32 hmaxu(packu x)
{
#if __ARM_NEON
  return vmaxvq_u32(x);
#endif
#if __AVX__
  x = maxu(x, castu(_mm256_permute_ps(castf(x), _MM_SHUFFLE(2, 3, 0, 1))));
  x = maxu(x, castu(_mm256_permute_ps(castf(x), _MM_SHUFFLE(1, 0, 3, 2))));
  x = maxu(x, _mm256_permute2f128_si256(x, x, _MM_SHUFFLE(0, 0, 0, 1)));
  return _mm_cvtsi128_si32(_mm256_castsi256_si128(x));
#endif
}

INLINE f32 hmin(packf x)
{
#if __ARM_NEON
  return vminvq_f32(x);
#endif
#if __AVX__
  x = min(x, _mm256_permute_ps(x, _MM_SHUFFLE(2, 3, 0, 1)));
  x = min(x, _mm256_permute_ps(x, _MM_SHUFFLE(1, 0, 3, 2)));
  x = min(x, _mm256_permute2f128_ps(x, x, _MM_SHUFFLE(0, 0, 0, 1)));
  return _mm_cvtss_f32(_mm256_castps256_ps128(x));
#endif
}

INLINE int all_leq(packf a, packf b)
{
#if __ARM_NEON
  packu m = vmvnq_u32(eq(min(a, b), a));
  uint32x2_t r = vshrn_n_u64(vreinterpretq_u64_u32(m), 16);
  return !vget_lane_u64(vreinterpret_u64_u32(r), 0);
#endif
#if __AVX__
  return 0xFF == _mm256_movemask_ps(_mm256_cmp_ps(a, b, _CMP_LE_OS));
#endif
}

INLINE void setf(packf *x, f32 v, int e)
{
  packf broad = dupf(v);
  int32_t m[2 * NUM_LANES] = {0};
  m[NUM_LANES] = -1;
  packu mask = loadu((void const *)(m + NUM_LANES - (e & (NUM_LANES - 1))));
  *x = blendf(castf(mask), broad, *x);
}

__attribute__((unused)) INLINE void setu(packu *x, u32 v, int e)
{
  packu broad = dupu(v);
  int32_t m[2 * NUM_LANES] = {0};
  m[NUM_LANES] = -1;
  packu mask = loadu((void const *)(m + NUM_LANES - (e & (NUM_LANES - 1))));
  *x = blendu(mask, broad, *x);
}

INLINE f32 getf(packf x, int e)
{
  f32 arr[NUM_LANES];
  storef(arr, x);
  return arr[e & (NUM_LANES - 1)];
}

INLINE u32 getu(packu x, int e)
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

INLINE packu pack_index(void)
{
#if __ARM_NEON
  return initu(0x0 << STEP_LANE_OFFSET, 0x1 << STEP_LANE_OFFSET,
               0x2 << STEP_LANE_OFFSET, 0x3 << STEP_LANE_OFFSET);
#endif
#if __AVX__
  return initu(0x0 << STEP_LANE_OFFSET, 0x1 << STEP_LANE_OFFSET,
               0x2 << STEP_LANE_OFFSET, 0x3 << STEP_LANE_OFFSET,
               0x4 << STEP_LANE_OFFSET, 0x5 << STEP_LANE_OFFSET,
               0x6 << STEP_LANE_OFFSET, 0x7 << STEP_LANE_OFFSET);
#endif
}

static inline int num_packs(int K)
{
  int r = (K - 1) / NUM_LANES + 1;
  return r < 2 ? 2 : r;
}

INLINE void acc(packf *acc_val, packf val, packu *acc_idx, packu idx, int save)
{
  packf x = min(*acc_val, val);
  // if (save) *acc_idx = blendu(eq(x, val), idx, *acc_idx);
  if (save) *acc_idx = blendu(eq(x, *acc_val), *acc_idx, idx);
  *acc_val = x;
}

INLINE void facc(f32 *acc_val, f32 val, u32 *acc_idx, u32 idx, int save)
{
  f32 x = fminf(*acc_val, val);
  if (save) *acc_idx = x == *acc_val ? *acc_idx : idx;
  *acc_val = x;
}

INLINE void hacc(f32 *hval, packf val, u32 *hidx, packu idx, int save)
{
  f32 x = hmin(val);
  if (save) *hidx = hmaxu(and(eq(val, dupf(x)), idx));
  *hval = x;
}

INLINE int core_pack(int k, int Q)   { return k % Q; }
INLINE int core_lane(int k, int Q)   { return k / Q; }
INLINE int imin(int a, int b)        { return a < b ? a : b; }

INLINE packf sum(packf a, packf b, packf c) { return add(add(a, b), c); }
INLINE int   timemap(int q, int t, int Q) { return t * Q + q; }

static inline void core_state_init(struct core_state *x, int t, int q,
                                   int Q)
{
  x[timemap(q, t, Q)].M = dupf(INFINITY);
  x[timemap(q, t, Q)].D = dupf(INFINITY);
  x[timemap(q, t, Q)].I = dupf(INFINITY);
}

static inline void core_trans_init(struct core_trans *x)
{
  x->BM = dupf(INFINITY);
  x->MM = dupf(INFINITY);
  x->MI = dupf(INFINITY);
  x->MD = dupf(INFINITY);
  x->IM = dupf(INFINITY);
  x->II = dupf(INFINITY);
  x->DM = dupf(INFINITY);
  x->DD = dupf(INFINITY);
}

static inline void emission_init(struct emission *x, int Q)
{
  for (int i = 0; i < DCP_ABC_TABLE_SIZE; ++i)
  {
    x->null[i] = INFINITY;
    x->background[i] = dupf(INFINITY);
  }

  for (int i = 0; i < DCP_ABC_TABLE_SIZE * Q; ++i)
    x->match[i] = dupf(INFINITY);
}

static inline void extr_state_init(struct extr_state *x, int t)
{
  x[t].S = INFINITY;
  x[t].N = INFINITY;
  x[t].B = INFINITY;

  x[t].J = INFINITY;

  x[t].E = INFINITY;
  x[t].C = INFINITY;
  x[t].T = INFINITY;
}

static inline void extr_trans_init(struct extr_trans *x)
{
  x->SN = INFINITY;
  x->NN = INFINITY;
  x->SB = INFINITY;
  x->NB = INFINITY;
  x->EB = INFINITY;
  x->JB = INFINITY;
  x->EJ = INFINITY;
  x->JJ = INFINITY;
  x->EC = INFINITY;
  x->CC = INFINITY;
  x->ET = INFINITY;
  x->CT = INFINITY;
}

static inline void prev_core_state_init(struct prev_core_state *x)
{
  x->M = dupu(step(STEP_BM, 1));
  x->D = dupu(step(STEP_MD, 0));
  x->I = dupu(step(STEP_MI, 1));
}

static inline void prev_extr_state_init(struct prev_extr_state *x)
{
  x->S = 0U;
  x->N = step(STEP_SN, 1);
  x->B = step(STEP_SB, 0);

  x->J = step(STEP_EJ, 1);

  x->E = step(STEP_ME, 0);
  x->C = step(STEP_EC, 1);
  x->T = step(STEP_ET, 0);
}

struct viterbi *viterbi_new(void)
{
  struct viterbi *x = aligned_alloc(ALIGNMENT, sizeof(struct viterbi));
  if (!x) return x;

  x->Q = x->maxQ = 0;
  x->emission.match = NULL;
  x->core_state = NULL;
  x->core_trans = NULL;
  x->prev_core_state = NULL;
  trellis_init(&x->trellis);

  return x;
}

void viterbi_del(struct viterbi const *x)
{
  if (x)
  {
    free(x->emission.match);
    free(x->core_state);
    free(x->core_trans);
    free(x->prev_core_state);
    trellis_cleanup((struct trellis *)&x->trellis);
    free((void *)x);
  }
}

int viterbi_setup(struct viterbi *x, int K)
{
  x->K = K;
  int Q = x->Q = num_packs(K);

  for (int t = 0; t < DCP_PAST_SIZE; ++t)
    extr_state_init(x->extr_state, t);

  if (Q > x->maxQ)
  {
    free(x->core_state);
    x->core_state = aligned_alloc(
        ALIGNMENT, sizeof(struct core_state[DCP_PAST_SIZE][Q]));
    if (!x->core_state) return 1;

    free(x->emission.match);
    x->emission.match =
        aligned_alloc(ALIGNMENT, sizeof(packf[DCP_ABC_TABLE_SIZE][Q]));
    if (!x->emission.match) return 1;

    free(x->core_trans);
    x->core_trans = aligned_alloc(ALIGNMENT, sizeof(struct core_trans[Q]));
    if (!x->core_trans) return 1;

    free(x->prev_core_state);
    x->prev_core_state =
        aligned_alloc(ALIGNMENT, sizeof(struct prev_core_state[Q]));
    if (!x->prev_core_state) return 1;

    x->maxQ = Q;
  }

  extr_trans_init(&x->extr_trans);
  for (int q = 0; q < Q; ++q)
    core_trans_init(&x->core_trans[q]);

  for (int q = 0; q < Q; ++q)
  {
    for (int t = 0; t < DCP_PAST_SIZE; ++t)
      core_state_init(x->core_state, t, q, Q);
  }

  emission_init(&x->emission, Q);

  return 0;
}

void viterbi_set_extr_trans(struct viterbi *x, enum extr_trans_id id,
                            f32 scalar)
{
  switch (id)
  {
  case EXTR_TRANS_RR: x->extr_trans.RR = scalar; break;
  case EXTR_TRANS_SN: x->extr_trans.SN = scalar; break;
  case EXTR_TRANS_NN: x->extr_trans.NN = scalar; break;
  case EXTR_TRANS_SB: x->extr_trans.SB = scalar; break;
  case EXTR_TRANS_NB: x->extr_trans.NB = scalar; break;
  case EXTR_TRANS_EB: x->extr_trans.EB = scalar; break;
  case EXTR_TRANS_JB: x->extr_trans.JB = scalar; break;
  case EXTR_TRANS_EJ: x->extr_trans.EJ = scalar; break;
  case EXTR_TRANS_JJ: x->extr_trans.JJ = scalar; break;
  case EXTR_TRANS_EC: x->extr_trans.EC = scalar; break;
  case EXTR_TRANS_CC: x->extr_trans.CC = scalar; break;
  case EXTR_TRANS_ET: x->extr_trans.ET = scalar; break;
  case EXTR_TRANS_CT: x->extr_trans.CT = scalar; break;
  default:
    __builtin_unreachable();
    break;
  }
}

void viterbi_set_core_trans(struct viterbi *x, enum core_trans_id id,
                            f32 scalar, int k)
{
  int q = core_pack(k, x->Q);
  int e = core_lane(k, x->Q);

  switch (id)
  {
  case CORE_TRANS_BM: setf(&x->core_trans[q].BM, scalar, e); break;
  case CORE_TRANS_MM: setf(&x->core_trans[q].MM, scalar, e); break;
  case CORE_TRANS_MI: setf(&x->core_trans[q].MI, scalar, e); break;
  case CORE_TRANS_MD: setf(&x->core_trans[q].MD, scalar, e); break;
  case CORE_TRANS_IM: setf(&x->core_trans[q].IM, scalar, e); break;
  case CORE_TRANS_II: setf(&x->core_trans[q].II, scalar, e); break;
  case CORE_TRANS_DM: setf(&x->core_trans[q].DM, scalar, e); break;
  case CORE_TRANS_DD: setf(&x->core_trans[q].DD, scalar, e); break;
  default:
    __builtin_unreachable();
    break;
  }
}

void viterbi_set_null(struct viterbi *x, f32 scalar, int code)
{
  x->emission.null[code] = scalar;
}

void viterbi_set_background(struct viterbi *x, f32 scalar, int code)
{
  x->emission.background[code] = dupf(scalar);
}

void viterbi_set_match(struct viterbi *x, f32 scalar, int k, int code)
{
  int q = core_pack(k, x->Q);
  int e = core_lane(k, x->Q);
  setf(&x->emission.match[code * x->Q + q], scalar, e);
}

static void before(struct trellis *, int K);
static void after(struct trellis *, int Q, int K, struct prev_extr_state *,
                  struct prev_core_state *);
static void dump_trellis(struct viterbi const *, int l);

INLINE f32 cost(struct viterbi *x, int L, int path, viterbi_code_fn code_fn,
                void *code_arg)
{
  struct emission const em = x->emission;

  struct extr_trans const xt = x->extr_trans;
  struct core_trans const *ct = x->core_trans;

  struct extr_state *xs = x->extr_state;
  struct core_state *cs = x->core_state;

  struct prev_extr_state *px = &x->prev_extr_state;
  struct prev_core_state *pc = x->prev_core_state;

#define xM(q, t) cs[timemap(q, t, Q)].M
#define xD(q, t) cs[timemap(q, t, Q)].D
#define xI(q, t) cs[timemap(q, t, Q)].I

  int Q = x->Q;

  // For l = 0
  xs[0].S = 0;
  xs[0].B = xt.SB;
  if (path) before(&x->trellis, x->K);
  for (int l = 1; l <= L; ++l)
  {
    extr_state_init(xs, imin(DCP_PAST_SIZE - 1, l));
    prev_extr_state_init(px);
    for (int q = 0; q < Q; ++q)
    {
      core_state_init(cs, imin(DCP_PAST_SIZE - 1, l), q, Q);
      prev_core_state_init(&pc[q]);
    }

    for (int t = imin(DCP_PAST_SIZE - 1, l); t > 0; --t)
    {
      int code = code_fn(l - t, t, code_arg);
      f32 nil = em.null[code];
      int a = t - 0;
      int z = t - 1;

      facc(&xs[a].N, xs[z].S + xt.SN + nil, &px->N, step(STEP_SN, t), path);
      facc(&xs[a].N, xs[z].N + xt.NN + nil, &px->N, step(STEP_NN, t), path);

      facc(&xs[a].B, xs[a].S + xt.SB, &px->B, step(STEP_SB, 0), path);
      facc(&xs[a].B, xs[a].N + xt.NB, &px->B, step(STEP_NB, 0), path);

      facc(&xs[a].J, xs[z].E + xt.EJ + nil, &px->J, step(STEP_EJ, t), path);
      facc(&xs[a].J, xs[z].J + xt.JJ + nil, &px->J, step(STEP_JJ, t), path);

      facc(&xs[a].C, xs[z].E + xt.EC + nil, &px->C, step(STEP_EC, t), path);
      facc(&xs[a].C, xs[z].C + xt.CC + nil, &px->C, step(STEP_CC, t), path);

      packf lastMz = shift(xM(Q - 1, z));
      packf lastDz = shift(xD(Q - 1, z));
      packf lastIz = shift(xI(Q - 1, z));
      packf lastMa = shift(xM(Q - 1, a));
      packf currBz = dupf(xs[z].B);
      packf E = dupf(INFINITY);
      packu prevE = dupu(0U);
      packf bg = em.background[code];
      for (int q = 0; q < Q; ++q)
      {
        packf ma = em.match[code * Q + q];
        packf Ma = xM(q, a);
        packf Da = xD(q, a);
        packf Ia = xI(q, a);
        packf BM = ct[q].BM;
        packf MM = ct[q].MM;
        packf IM = ct[q].IM;
        packf DM = ct[q].DM;
        packf MI = ct[q].MI;
        packf II = ct[q].II;
        packf MD = ct[q].MD;

        acc(&Ma, sum(currBz, BM, ma), &pc[q].M, dupu(step(STEP_BM, t)), path);
        acc(&Ma, sum(lastMz, MM, ma), &pc[q].M, dupu(step(STEP_MM, t)), path);
        acc(&Ma, sum(lastIz, IM, ma), &pc[q].M, dupu(step(STEP_IM, t)), path);
        acc(&Ma, sum(lastDz, DM, ma), &pc[q].M, dupu(step(STEP_DM, t)), path);

        lastMz = xM(q, z);
        lastDz = xD(q, z);
        lastIz = xI(q, z);

        acc(&Ia, sum(lastIz, II, bg), &pc[q].I, dupu(step(STEP_II, t)), path);
        acc(&Ia, sum(lastMz, MI, bg), &pc[q].I, dupu(step(STEP_MI, t)), path);

        acc(&Da, add(lastMa, MD), &pc[q].D, dupu(step(STEP_MD, 0)), path);

        acc(&E, Ma, &prevE, dupu(step(STEP_ME, q)), path);
        acc(&E, Da, &prevE, dupu(step(STEP_DE, q)), path);

        lastMa = Ma;

        xM(q, a) = lastMz;
        xD(q, a) = lastDz;
        xI(q, a) = lastIz;
        xM(q, z) = Ma;
        xD(q, z) = Da;
        xI(q, z) = Ia;
      }
      {
        lastMa = shift(lastMa);
        packf MD = ct[0].MD;
        acc(&xD(0, z), add(lastMa, MD), &pc[0].D, dupu(step(STEP_MD, 0)), path);
        acc(&E, xD(0, z), &prevE, dupu(step(STEP_DE, 0)), path);
        prevE = or(prevE, pack_index());
        hacc(&xs[a].E, E, &px->E, prevE, path);
      }

      packf lastD0 = shift(xD(Q - 1, z));
      for (int q = 0; q < Q; ++q)
      {
        packf DD = ct[q].DD;
        acc(&xD(q, z), add(lastD0, DD), &pc[q].D, dupu(step(STEP_DD, 0)), path);
        lastD0 = xD(q, z);
      }

      int q = 0;
      do
      {
        lastD0 = shift(lastD0);
        for (q = 0; q < Q; ++q)
        {
          packf x = add(lastD0, ct[q].DD);
          if (all_leq(xD(q, z), x)) break;
          acc(&xD(q, z), x, &pc[q].D, dupu(step(STEP_DD, 0)), path);
          lastD0 = xD(q, z);
        }
      } while (q == Q);

      facc(&xs[a].B, xs[a].E + xt.EB, &px->B, step(STEP_EB, 0), path);
      facc(&xs[a].B, xs[a].J + xt.JB, &px->B, step(STEP_JB, 0), path);

      facc(&xs[a].T, xs[a].E + xt.ET, &px->T, step(STEP_ET, 0), path);
      facc(&xs[a].T, xs[a].C + xt.CT, &px->T, step(STEP_CT, 0), path);

      struct extr_state tmp = xs[z];
      xs[z] = xs[a];
      xs[a] = tmp;
    }
    if (path) after(&x->trellis, Q, x->K, px, pc);
  }

#undef xM
#undef xD
#undef xI

  return xs[0].T;
}

static void before(struct trellis *tr, int K)
{
  trellis_seek_xnode(tr, 0);
  trellis_seek_node(tr, 0, 0);

  trellis_clear_xnode(tr);
  trellis_set(tr, STATE_N, 0);
  trellis_set(tr, STATE_B, 0); // SB
  trellis_clear_node(tr);
  trellis_set(tr, STATE_M, 0);
  for (int k = 0; k + 1 < K; ++k)
  {
    trellis_set(tr, STATE_I, 0);
    trellis_next_node(tr);
    trellis_clear_node(tr);
    trellis_set(tr, STATE_M, 0);
    trellis_set(tr, STATE_D, 0);
  }
  trellis_next_node(tr);
  trellis_set(tr, STATE_E, 0);
  trellis_set(tr, STATE_J, 0);
  trellis_set(tr, STATE_C, 0);
  trellis_set(tr, STATE_T, 0);
  trellis_next_xnode(tr);

  trellis_seek_xnode(tr, 1);
  trellis_seek_node(tr, 1, 0);
}

static void after(struct trellis *tr, int Q, int K, struct prev_extr_state *px,
                  struct prev_core_state *pc)
{
  trellis_clear_xnode(tr);
  if (STEP_SN & px->N) trellis_set(tr, STATE_N, 0 + step_data(px->N) - 1);
  if (STEP_NN & px->N) trellis_set(tr, STATE_N, 5 + step_data(px->N) - 1);

  if (STEP_SB & px->B) trellis_set(tr, STATE_B, step_data(px->B) + 0);
  if (STEP_NB & px->B) trellis_set(tr, STATE_B, step_data(px->B) + 1);
  if (STEP_EB & px->B) trellis_set(tr, STATE_B, step_data(px->B) + 2);
  if (STEP_JB & px->B) trellis_set(tr, STATE_B, step_data(px->B) + 3);

  if (tr) trellis_clear_node(tr);
  {
    int q = core_pack(0, Q);
    int e = core_lane(0, Q);
    u32 M = getu(pc[q].M, e);
    if (tr && STEP_BM & M) trellis_set(tr, STATE_M, 0 + step_data(M) - 1);
  }

  for (int k = 0; k + 1 < K; ++k)
  {
    int q = core_pack(k, Q);
    int e = core_lane(k, Q);
    u32 I = getu(pc[q].I, e);
    int n = k + 1;
    int qn = core_pack(n, Q);
    int en = core_lane(n, Q);
    u32 M = getu(pc[qn].M, en);
    u32 D = getu(pc[qn].D, en);
    if (STEP_MI & I) trellis_set(tr, STATE_I, 0 + step_data(I) - 1);
    if (STEP_II & I) trellis_set(tr, STATE_I, 5 + step_data(I) - 1);
    trellis_next_node(tr);
    trellis_clear_node(tr);

    if (STEP_BM & M) trellis_set(tr, STATE_M, 0 + step_data(M) - 1);
    if (STEP_MM & M) trellis_set(tr, STATE_M, 5 + step_data(M) - 1);
    if (STEP_IM & M) trellis_set(tr, STATE_M, 10 + step_data(M) - 1);
    if (STEP_DM & M) trellis_set(tr, STATE_M, 15 + step_data(M) - 1);

    if (STEP_MD & D) trellis_set(tr, STATE_D, 0);
    if (STEP_DD & D) trellis_set(tr, STATE_D, 1);
  }
  if (tr) trellis_next_node(tr);

  int q = step_data(px->E);
  int e = step_lane(px->E);
  int k = e * Q + q;
  if (STEP_ME & px->E) trellis_set(tr, STATE_E, 2 * k + 0);
  if (STEP_DE & px->E) trellis_set(tr, STATE_E, 2 * k + 1);

  if (STEP_EJ & px->J) trellis_set(tr, STATE_J, 0 + step_data(px->J) - 1);
  if (STEP_JJ & px->J) trellis_set(tr, STATE_J, 5 + step_data(px->J) - 1);

  if (STEP_EC & px->C) trellis_set(tr, STATE_C, 0 + step_data(px->C) - 1);
  if (STEP_CC & px->C) trellis_set(tr, STATE_C, 5 + step_data(px->C) - 1);

  if (STEP_ET & px->T) trellis_set(tr, STATE_T, 0 + step_data(px->T));
  if (STEP_CT & px->T) trellis_set(tr, STATE_T, 1 + step_data(px->T));

  trellis_next_xnode(tr);

  // if (path) dump_trellis(x, l);
}

float viterbi_null(struct viterbi *x, int L, viterbi_code_fn fn, void *arg)
{
  float RR = x->extr_trans.RR;
  struct emission const em = x->emission;
  float R[DCP_PAST_SIZE] = {};
  for (int i = 0; i < DCP_PAST_SIZE; ++i)
    R[i] = INFINITY;
  R[0] = -RR;
  for (int l = 1; l <= L; ++l)
  {
    R[imin(DCP_PAST_SIZE - 1, l)] = INFINITY;
    for (int t = imin(DCP_PAST_SIZE - 1, l); t > 0; --t)
    {
      int code = fn(l - t, t, arg);
      f32 nil = em.null[code];
      int a = t - 0;
      int z = t - 1;
      float tmp = fminf(R[a], R[z] + RR + nil);
      R[a] = R[z];
      R[z] = tmp;
    }
  }
  return R[0];
}

f32 viterbi_cost(struct viterbi *x, int L, viterbi_code_fn fn, void *arg)
{
  return cost(x, L, 0, fn, arg);
}

int viterbi_path(struct viterbi *x, int L, viterbi_code_fn fn, void *arg)
{
  int rc = trellis_setup(&x->trellis, x->K, L);
  if (rc) return rc;
  cost(x, L, 1, fn, arg);
  return 0;
}

struct trellis *viterbi_trellis(struct viterbi *x)
{
  return &x->trellis;
}

__attribute__((unused)) static void dump_trellis(struct viterbi const *x, int l)
{
  struct prev_extr_state const *px = &x->prev_extr_state;
  struct prev_core_state const *pc = x->prev_core_state;

  if      (px->N & STEP_SN) printf("SN=%d ", step_data(px->N));
  else if (px->N & STEP_NN) printf("NN=%d ", step_data(px->N));
  else                      printf("?N=? ");

  for (int k = 0; k < x->K; ++k)
  {
#define CORE(q, name, lane) getu(pc[q].name, lane)
#define DATA(q, name, lane) step_data(CORE(q, M, e))
    int q = core_pack(k, x->Q);
    int e = core_lane(k, x->Q);
    if      (CORE(q, M, e) & STEP_BM) printf("BM=%d ", DATA(q, M, e));
    else if (CORE(q, M, e) & STEP_MM) printf("MM=%d ", DATA(q, M, e));
    else if (CORE(q, M, e) & STEP_IM) printf("IM=%d ", DATA(q, M, e));
    else if (CORE(q, M, e) & STEP_DM) printf("DM=%d ", DATA(q, M, e));
    else    printf("?M=? ");

    if      (CORE(q, I, e) & STEP_MI) printf("MI=%d ", DATA(q, I, e));
    else if (CORE(q, I, e) & STEP_II) printf("II=%d ", DATA(q, I, e));
    else    printf("?I=? ");

    if      (CORE(q, D, e) & STEP_MD) printf("MD=%d ", DATA(q, D, e));
    else if (CORE(q, D, e) & STEP_DD) printf("DD=%d ", DATA(q, D, e));
    else    printf("?D=? ");
#undef DATA
#undef CORE
  }

  if      (px->B & STEP_SB) printf("SB=%d ", step_data(px->B));
  else if (px->B & STEP_NB) printf("NB=%d ", step_data(px->B));
  else if (px->B & STEP_EB) printf("EB=%d ", step_data(px->B));
  else if (px->B & STEP_JB) printf("JB=%d ", step_data(px->B));
  else                      printf("?B=? ");

  {
    int q = step_data(px->E);
    int e = step_lane(px->E);
    int k = e * x->Q + q;
    if      (px->E & STEP_ME) printf("ME=%d ", k);
    else if (px->E & STEP_DE) printf("DE=%d ", k);
    else                      printf("?E=? ");
  }

  if      (px->J & STEP_EJ) printf("EJ=%d ", step_data(px->J));
  else if (px->J & STEP_JJ) printf("JJ=%d ", step_data(px->J));
  else                      printf("?J=? ");

  if      (px->C & STEP_EC) printf("EC=%d ", step_data(px->C));
  else if (px->C & STEP_CC) printf("CC=%d ", step_data(px->C));
  else                      printf("?C=? ");

  if      (px->T & STEP_ET) printf("ET=%d ", step_data(px->T));
  else if (px->T & STEP_CT) printf("CT=%d ", step_data(px->T));
  else                      printf("?T=? ");
  printf("l=%d K=%d\n", l, x->K);
}
