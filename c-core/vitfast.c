#include "vitfast.h"
#include <math.h>
#include <stdlib.h>

#if !defined(__AVX__) && !defined(__ARM_NEON)
  #error "We require either AVX or NEON feature."
#endif

#if __ARM_NEON
  #include <arm_neon.h>
  typedef float32x4_t pack;
  #define NUM_LANES 4
  #define ALIGNMENT 16
#endif

#if __AVX__
  #include <immintrin.h>
  typedef __m256 pack;
  #define NUM_LANES 8
  #define ALIGNMENT 32
#endif

struct emission
{
  float null[VITFAST_TABLE_SIZE];
  pack background[VITFAST_TABLE_SIZE];
  pack *match;
};

struct extr_trans
{
  float SN;
  float NN;

  float SB;
  float NB;
  float EB;
  float JB;

  float EJ;
  float JJ;

  float EC;
  float CC;

  float ET;
  float CT;
};

struct core_trans
{
  pack BM;
  pack MM;
  pack MI;
  pack MD;
  pack IM;
  pack II;
  pack DM;
  pack DD;
};

struct extr_state
{
  // Prefix
  float S;
  float N;
  float B;

  // Infix
  float J;

  // Suffix
  float E;
  float C;
  float T;
};

struct core_state
{
  pack M;
  pack D;
  pack I;
};

struct vitfast
{
  int Q;
  int maxQ;
  struct extr_state extr_state[VITFAST_TIME_FRAME];
  struct core_state *core_state;

  struct emission emission;

  struct extr_trans extr_trans;
  struct core_trans *core_trans;
};

struct vitfast *vitfast_new(void)
{
  struct vitfast *x = malloc(sizeof(struct vitfast));
  if (!x) return x;

  x->Q = x->maxQ = 0;
  x->core_state = NULL;
  x->core_trans = NULL;
  x->emission.match = NULL;

  return x;
}

#define INLINE static inline __attribute__((always_inline))

static inline void extr_trans_init(struct extr_trans *);
static inline void core_trans_init(struct core_trans *);
static inline void extr_state_init(struct extr_state *, int t);
static inline void core_state_init(struct core_state *, int t, int q, int Q);
static inline void emission_init(struct emission *, int Q);

int vitfast_setup(struct vitfast *x, int K)
{
  int Q = x->Q = vitfast_num_packs(K);

  for (int t = 0; t < VITFAST_TIME_FRAME; ++t)
    extr_state_init(x->extr_state, t);

  if (Q > x->maxQ)
  {
    free(x->core_state);
    x->core_state =
        aligned_alloc(ALIGNMENT, sizeof(struct core_state[VITFAST_TIME_FRAME][Q]));
    if (!x->core_state) return 1;

    free(x->emission.match);
    x->emission.match = aligned_alloc(ALIGNMENT, sizeof(pack[VITFAST_TABLE_SIZE][Q]));
    if (!x->emission.match) return 1;

    free(x->core_trans);
    x->core_trans = aligned_alloc(ALIGNMENT, sizeof(struct core_trans[Q]));
    if (!x->core_trans) return 1;

    x->maxQ = Q;
  }

  extr_trans_init(&x->extr_trans);
  for (int q = 0; q < Q; ++q)
    core_trans_init(&x->core_trans[q]);

  for (int q = 0; q < Q; ++q)
  {
    for (int t = 0; t < VITFAST_TIME_FRAME; ++t)
      core_state_init(x->core_state, t, q, Q);
  }

  emission_init(&x->emission, Q);

  return 0;
}

void vitfast_del(struct vitfast const *x)
{
  if (x)
  {
    free(x->core_state);
    free(x->core_trans);
    free(x->emission.match);
    free((void *)x);
  }
}

#if __ARM_NEON
INLINE pack  min(pack a, pack b) { return vminq_f32(a, b); }
INLINE pack  add(pack a, pack b) { return vaddq_f32(a, b); }
INLINE pack  dup(float x)        { return vdupq_n_f32(x); }
INLINE pack  shift(pack x)       { return vextq_f32(dup(INFINITY), x, NUM_LANES - 1); }
INLINE float hmin(pack x)        { return vminvq_f32(x); }
#endif

#if __AVX__
INLINE pack min(pack a, pack b) { return _mm256_min_ps(a, b); }
INLINE pack add(pack a, pack b) { return _mm256_add_ps(a, b); }
INLINE pack dup(float x)        { return _mm256_set1_ps(x); }

INLINE pack shift(pack x)
{
  x = _mm256_permutevar8x32_ps(x, _mm256_set_epi32(6, 5, 4, 3, 2, 1, 0, 7));
  return _mm256_blend_ps(x, dup(INFINITY), _MM_SHUFFLE(0, 0, 0, 1));
}

INLINE float hmin(pack x)
{
  x = _mm256_min_ps(x, _mm256_permute_ps(x, _MM_SHUFFLE(2, 3, 0, 1)));
  x = _mm256_min_ps(x, _mm256_permute_ps(x, _MM_SHUFFLE(1, 0, 3, 2)));
  x = _mm256_min_ps(x, _mm256_permute2f128_ps(x, x, _MM_SHUFFLE(0, 0, 0, 1)));
  return _mm_cvtss_f32(_mm256_castps256_ps128(x));
}
#endif

INLINE int   time_map(int q, int t, int Q) { return t * Q + q; }
INLINE int   imin(int a, int b)            { return a < b ? a : b; }
INLINE pack  sum(pack a, pack b, pack c)   { return add(add(a, b), c); }
INLINE int   core_pack(int k, int Q)       { return k % Q; }
INLINE int   core_lane(int k, int Q)       { return k / Q; }
INLINE int   all_leq(pack a, pack b);
INLINE void  set(pack *, float scalar, int e);

float vitfast_cost(struct vitfast *x, int L, vitfast_code_fn code_fn,
                   void *code_arg)
{
  struct emission const em = x->emission;

  struct extr_trans const xt = x->extr_trans;
  struct core_trans const *ct = x->core_trans;

  struct extr_state *xs = x->extr_state;
  struct core_state *cs = x->core_state;

#define xM(q, t) cs[time_map(q, t, Q)].M
#define xD(q, t) cs[time_map(q, t, Q)].D
#define xI(q, t) cs[time_map(q, t, Q)].I

  int Q = x->Q;

  // For l = 0
  xs[0].S = 0;
  xs[0].B = xt.SB;

  for (int l = 1; l <= L; ++l)
  {
    extr_state_init(xs, imin((VITFAST_TIME_FRAME - 1), l));
    for (int q = 0; q < Q; ++q)
      core_state_init(cs, imin((VITFAST_TIME_FRAME - 1), l), q, Q);

    for (int t = imin((VITFAST_TIME_FRAME - 1), l); t > 0; --t)
    {
      int code = code_fn(l - t, t, code_arg);
      float nil = em.null[code];
      int a = t;
      int z = t - 1;

      xs[a].N = fminf(xs[a].N, xs[z].S + xt.SN + nil);
      xs[a].N = fminf(xs[a].N, xs[z].N + xt.NN + nil);

      xs[a].B = fminf(xs[a].B, xs[a].S + xt.SB);
      xs[a].B = fminf(xs[a].B, xs[a].N + xt.NB);

      xs[a].J = fminf(xs[a].J, xs[z].E + xt.EJ + nil);
      xs[a].J = fminf(xs[a].J, xs[z].J + xt.JJ + nil);

      xs[a].C = fminf(xs[a].C, xs[z].E + xt.EC + nil);
      xs[a].C = fminf(xs[a].C, xs[z].C + xt.CC + nil);

      pack lastMz = shift(xM(Q - 1, z));
      pack lastDz = shift(xD(Q - 1, z));
      pack lastIz = shift(xI(Q - 1, z));
      pack lastMa = shift(xM(Q - 1, a));
      pack currBz = dup(xs[z].B);
      pack accumE = dup(INFINITY);
      pack bg = em.background[code];
      for (int q = 0; q < Q; ++q)
      {
        pack ma = em.match[code * Q + q];
        pack xMa = xM(q, a);
        pack xDa = xD(q, a);
        pack xIa = xI(q, a);

        xMa = min(xMa, sum(currBz, ct[q].BM, ma));
        xMa = min(xMa, sum(lastMz, ct[q].MM, ma));
        xMa = min(xMa, sum(lastIz, ct[q].IM, ma));
        xMa = min(xMa, sum(lastDz, ct[q].DM, ma));

        lastMz = xM(q, z);
        lastDz = xD(q, z);
        lastIz = xI(q, z);

        xIa = min(xIa, sum(lastMz, ct[q].MI, bg));
        xIa = min(xIa, sum(lastIz, ct[q].II, bg));

        xDa = min(xDa, add(lastMa, ct[q].MD));

        accumE = min(accumE, min(xMa, xDa));

        lastMa = xMa;

        xM(q, a) = lastMz;
        xD(q, a) = lastDz;
        xI(q, a) = lastIz;
        xM(q, z) = xMa;
        xD(q, z) = xDa;
        xI(q, z) = xIa;
      }
      xD(0, z) = min(xD(0, z), add(shift(lastMa), ct[0].MD));
      xs[a].E = hmin(min(accumE, xD(0, z)));

      pack lastD0 = shift(xD(Q - 1, z));
      for (int q = 0; q < Q; ++q)
        lastD0 = xD(q, z) = min(xD(q, z), add(lastD0, ct[q].DD));

      int q = 0;
      do
      {
        lastD0 = shift(lastD0);
        for (q = 0; q < Q; ++q)
        {
          pack x = add(lastD0, ct[q].DD);
          if (all_leq(xD(q, z), x)) break;
          lastD0 = xD(q, z) = min(xD(q, z), x);
        }
      } while (q == Q);

      xs[a].B = fminf(xs[a].B, xs[a].E + xt.EB);
      xs[a].B = fminf(xs[a].B, xs[a].J + xt.JB);

      xs[a].T = fminf(xs[a].T, xs[a].E + xt.ET);
      xs[a].T = fminf(xs[a].T, xs[a].C + xt.CT);

      struct extr_state tmp = xs[z];
      xs[z] = xs[a];
      xs[a] = tmp;
    }
  }

#undef xM
#undef xD
#undef xI

  return xs[0].T;
}

void vitfast_set_extr_trans(struct vitfast *x, enum extr_trans_id id,
                            float scalar)
{
  switch (id)
  {
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

void vitfast_set_core_trans(struct vitfast *x, enum core_trans_id id,
                       float scalar, int k)
{
  int q = core_pack(k, x->Q);
  int e = core_lane(k, x->Q);

  switch (id)
  {
  case CORE_TRANS_BM: set(&x->core_trans[q].BM, scalar, e); break;
  case CORE_TRANS_MM: set(&x->core_trans[q].MM, scalar, e); break;
  case CORE_TRANS_MI: set(&x->core_trans[q].MI, scalar, e); break;
  case CORE_TRANS_MD: set(&x->core_trans[q].MD, scalar, e); break;
  case CORE_TRANS_IM: set(&x->core_trans[q].IM, scalar, e); break;
  case CORE_TRANS_II: set(&x->core_trans[q].II, scalar, e); break;
  case CORE_TRANS_DM: set(&x->core_trans[q].DM, scalar, e); break;
  case CORE_TRANS_DD: set(&x->core_trans[q].DD, scalar, e); break;
  default:
    __builtin_unreachable();
    break;
  }
}

void vitfast_set_null(struct vitfast *x, float scalar, int code)
{
  x->emission.null[code] = scalar;
}

void vitfast_set_background(struct vitfast *x, float scalar, int code)
{
  x->emission.background[code] = dup(scalar);
}

void vitfast_set_match(struct vitfast *x, float scalar, int k, int code)
{
  int q = core_pack(k, x->Q);
  int e = core_lane(k, x->Q);
  set(&x->emission.match[code * x->Q + q], scalar, e);
}

int vitfast_num_packs(int K)
{
  int r = (K - 1) / NUM_LANES + 1;
  return r < 2 ? 2 : r;
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

static inline void core_trans_init(struct core_trans *x)
{
  x->BM = dup(INFINITY);
  x->MM = dup(INFINITY);
  x->MI = dup(INFINITY);
  x->MD = dup(INFINITY);
  x->IM = dup(INFINITY);
  x->II = dup(INFINITY);
  x->DM = dup(INFINITY);
  x->DD = dup(INFINITY);
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

static inline void core_state_init(struct core_state *x, int t, int q,
                                   int Q)
{
  x[time_map(q, t, Q)].M = dup(INFINITY);
  x[time_map(q, t, Q)].D = dup(INFINITY);
  x[time_map(q, t, Q)].I = dup(INFINITY);
}

static inline void emission_init(struct emission *x, int Q)
{
  for (int i = 0; i < VITFAST_TABLE_SIZE; ++i)
  {
    x->null[i] = INFINITY;
    x->background[i] = dup(INFINITY);
  }

  for (int i = 0; i < VITFAST_TABLE_SIZE * Q; ++i)
    x->match[i] = dup(INFINITY);
}

INLINE int all_leq(pack a, pack b)
{
#if __ARM_NEON
  uint32x4_t m = vmvnq_u32(vceqq_f32(min(a, b), a));
  uint32x2_t r = vshrn_n_u64(vreinterpretq_u64_u32(m), 16);
  return !vget_lane_u64(vreinterpret_u64_u32(r), 0);
#endif

#if __AVX__
  return 0xFF == _mm256_movemask_ps(_mm256_cmp_ps(a, b, _CMP_LE_OS));
#endif
}

INLINE void set(pack *x, float scalar, int e)
{
#if __ARM_NEON
  switch (e)
  {
  case 0: *x = vsetq_lane_f32(scalar, *x, 0); break;
  case 1: *x = vsetq_lane_f32(scalar, *x, 1); break;
  case 2: *x = vsetq_lane_f32(scalar, *x, 2); break;
  case 3: *x = vsetq_lane_f32(scalar, *x, 3); break;
  default:
    __builtin_unreachable();
    break;
  }
#endif

#if __AVX__
  switch (e)
  {
  case 0: *x = _mm256_blend_ps(*x, _mm256_set1_ps(scalar), 1 << 0); break;
  case 1: *x = _mm256_blend_ps(*x, _mm256_set1_ps(scalar), 1 << 1); break;
  case 2: *x = _mm256_blend_ps(*x, _mm256_set1_ps(scalar), 1 << 2); break;
  case 3: *x = _mm256_blend_ps(*x, _mm256_set1_ps(scalar), 1 << 3); break;
  case 4: *x = _mm256_blend_ps(*x, _mm256_set1_ps(scalar), 1 << 4); break;
  case 5: *x = _mm256_blend_ps(*x, _mm256_set1_ps(scalar), 1 << 5); break;
  case 6: *x = _mm256_blend_ps(*x, _mm256_set1_ps(scalar), 1 << 6); break;
  case 7: *x = _mm256_blend_ps(*x, _mm256_set1_ps(scalar), 1 << 7); break;
  default:
    __builtin_unreachable();
    break;
  }
#endif
}
