#include "vitref.h"
#include "sample.h"
#include <math.h>
#include <stdlib.h>

#define INLINE static inline __attribute__((always_inline))

struct emission
{
  float null[VITREF_TABLE_SIZE];
  float background[VITREF_TABLE_SIZE];
  float *match;
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
  float BM;
  float MM;
  float MI;
  float MD;
  float IM;
  float II;
  float DM;
  float DD;
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
  float M;
  float D;
  float I;
};

struct vitref
{
  int K;
  struct extr_state extr_state[VITREF_TIME_FRAME];
  struct core_state *core_state;

  struct emission emission;

  struct extr_trans extr_trans;
  struct core_trans *core_trans;
};

struct vitref *vitref_new(void)
{
  struct vitref *x = malloc(sizeof(struct vitref));
  if (!x) return x;

  x->K = 0;
  x->core_state = NULL;
  x->core_trans = NULL;
  x->emission.match = NULL;

  return x;
}

static inline void extr_trans_init(struct extr_trans *);
static inline void core_trans_init(struct core_trans *);
static inline void extr_state_init(struct extr_state *, int t);
static inline void core_state_init(struct core_state *, int t, int k, int K);
static inline void emission_init(struct emission *, int K);

int vitref_setup(struct vitref *x, int K)
{
  x->K = K;

  for (int t = 0; t < VITREF_TIME_FRAME; ++t)
    extr_state_init(x->extr_state, t);

  free(x->core_state);
  x->core_state = malloc(sizeof(struct core_state[VITREF_TIME_FRAME][K]));
  if (!x->core_state) return 1;

  free(x->emission.match);
  x->emission.match = malloc(sizeof(float[VITREF_TABLE_SIZE][K]));
  if (!x->emission.match) return 1;

  free(x->core_trans);
  x->core_trans = malloc(sizeof(struct core_trans[K]));
  if (!x->core_trans) return 1;

  extr_trans_init(&x->extr_trans);
  for (int k = 0; k < K; ++k)
    core_trans_init(&x->core_trans[k]);

  for (int k = 0; k < K; ++k)
  {
    for (int t = 0; t < VITREF_TIME_FRAME; ++t)
      core_state_init(x->core_state, t, k, K);
  }

  emission_init(&x->emission, K);

  return 0;
}

float vitref_get_extr_trans(struct vitref const *x,
                            enum extr_trans_id id)
{
  switch (id)
  {
  case EXTR_TRANS_SN: return x->extr_trans.SN; break;
  case EXTR_TRANS_NN: return x->extr_trans.NN; break;
  case EXTR_TRANS_SB: return x->extr_trans.SB; break;
  case EXTR_TRANS_NB: return x->extr_trans.NB; break;
  case EXTR_TRANS_EB: return x->extr_trans.EB; break;
  case EXTR_TRANS_JB: return x->extr_trans.JB; break;
  case EXTR_TRANS_EJ: return x->extr_trans.EJ; break;
  case EXTR_TRANS_JJ: return x->extr_trans.JJ; break;
  case EXTR_TRANS_EC: return x->extr_trans.EC; break;
  case EXTR_TRANS_CC: return x->extr_trans.CC; break;
  case EXTR_TRANS_ET: return x->extr_trans.ET; break;
  case EXTR_TRANS_CT: return x->extr_trans.CT; break;
  default:
    __builtin_unreachable();
    break;
  }
}

float vitref_get_core_trans(struct vitref const *x,
                            enum core_trans_id id, int k)
{
  switch (id)
  {
  case CORE_TRANS_BM: return x->core_trans[k].BM; break;
  case CORE_TRANS_MM: return x->core_trans[k].MM; break;
  case CORE_TRANS_MI: return x->core_trans[k].MI; break;
  case CORE_TRANS_MD: return x->core_trans[k].MD; break;
  case CORE_TRANS_IM: return x->core_trans[k].IM; break;
  case CORE_TRANS_II: return x->core_trans[k].II; break;
  case CORE_TRANS_DM: return x->core_trans[k].DM; break;
  case CORE_TRANS_DD: return x->core_trans[k].DD; break;
  default:
    __builtin_unreachable();
    break;
  }
}

float vitref_get_null(struct vitref const *x, int code)
{
  return x->emission.null[code];
}

float vitref_get_background(struct vitref const *x, int code)
{
  return x->emission.background[code];
}

float vitref_get_match(struct vitref const *x, int k, int code)
{
  return x->emission.match[code * x->K + k];
}

INLINE int   time_map(int k, int t, int K) { return t * K + k; }
INLINE int   imin(int a, int b)            { return a < b ? a : b; }
INLINE float min(float a, float b)         { return fminf(a, b); }

static inline void core_advance(struct core_state *, int K);
static inline void extr_advance(struct extr_state *);

float vitref_cost(struct vitref *x, int L, vitref_code_fn code_fn,
                  void *code_arg)
{
  struct emission const em = x->emission;

  struct extr_trans const xt = x->extr_trans;
  struct core_trans const *ct = x->core_trans;

  struct extr_state *xs = x->extr_state;
  struct core_state *cs = x->core_state;

#define xM(k, t) cs[time_map(k, t, K)].M
#define xD(k, t) cs[time_map(k, t, K)].D
#define xI(k, t) cs[time_map(k, t, K)].I

  int K = x->K;

  // For l = 0
  xs[1].S = 0;
  xs[1].B = xt.SB;

  for (int l = 1; l <= L; ++l)
  {
    for (int t = imin((VITREF_TIME_FRAME - 1), l); t > 0; --t)
    {
      int code = code_fn(l - t, t, code_arg);
      float nil = em.null[code];

      xs[0].N = min(xs[0].N, xs[t].S + xt.SN + nil);
      xs[0].N = min(xs[0].N, xs[t].N + xt.NN + nil);

      xs[0].B = min(xs[0].B, xs[0].S + xt.SB);
      xs[0].B = min(xs[0].B, xs[0].N + xt.NB);

      xs[0].J = min(xs[0].J, xs[t].E + xt.EJ + nil);
      xs[0].J = min(xs[0].J, xs[t].J + xt.JJ + nil);

      xs[0].C = min(xs[0].C, xs[t].E + xt.EC + nil);
      xs[0].C = min(xs[0].C, xs[t].C + xt.CC + nil);

      float lastMt = INFINITY;
      float lastDt = INFINITY;
      float lastIt = INFINITY;
      float lastM0 = INFINITY;
      float lastD0 = INFINITY;
      float accumE = INFINITY;
      float bg = em.background[code];
      for (int k = 0; k < K; ++k)
      {
        float ma = em.match[code * K + k];

        xM(k, 0) = min(xM(k, 0), xs[t].B + ct[k].BM + ma);
        xM(k, 0) = min(xM(k, 0), lastMt + ct[k].MM + ma);
        xM(k, 0) = min(xM(k, 0), lastIt + ct[k].IM + ma);
        xM(k, 0) = min(xM(k, 0), lastDt + ct[k].DM + ma);

        xI(k, 0) = min(xI(k, 0), xM(k, t) + ct[k].MI + bg);
        xI(k, 0) = min(xI(k, 0), xI(k, t) + ct[k].II + bg);

        xD(k, 0) = min(xD(k, 0), lastM0 + ct[k].MD);
        xD(k, 0) = min(xD(k, 0), lastD0 + ct[k].DD);

        accumE = min(accumE, xM(k, 0));
        accumE = min(accumE, xD(k, 0));

        lastMt = xM(k, t);
        lastDt = xD(k, t);
        lastIt = xI(k, t);
        lastM0 = xM(k, 0);
        lastD0 = xD(k, 0);
      }
      xs[0].E = accumE;

      xs[0].B = min(xs[0].B, xs[0].E + xt.EB);
      xs[0].B = min(xs[0].B, xs[0].J + xt.JB);

      xs[0].T = min(xs[0].T, xs[0].E + xt.ET);
      xs[0].T = min(xs[0].T, xs[0].C + xt.CT);
    }
    core_advance(cs, K);
    extr_advance(xs);
  }

#undef xM
#undef xD
#undef xI

  return xs[1].T;
}

void vitref_del(struct vitref const *x)
{
  if (x)
  {
    free(x->core_state);
    free(x->core_trans);
    free(x->emission.match);
    free((void *)x);
  }
}

void vitref_sample(struct vitref *x, int seed)
{
  srand(seed);
  x->extr_trans.SN = sample_float();
  x->extr_trans.NN = sample_float();

  x->extr_trans.SB = sample_float();
  x->extr_trans.NB = sample_float();
  x->extr_trans.EB = sample_float();
  x->extr_trans.JB = sample_float();

  x->extr_trans.EJ = sample_float();
  x->extr_trans.JJ = sample_float();

  x->extr_trans.EC = sample_float();
  x->extr_trans.CC = sample_float();

  x->extr_trans.ET = sample_float();
  x->extr_trans.CT = sample_float();

  for (int k = 0; k < x->K; ++k)
  {
    x->core_trans[k].BM = sample_float();
    x->core_trans[k].MM = sample_float();
    x->core_trans[k].MI = sample_float();
    x->core_trans[k].MD = sample_float();
    x->core_trans[k].IM = sample_float();
    x->core_trans[k].II = sample_float();
    x->core_trans[k].DM = sample_float();
    x->core_trans[k].DD = sample_float();
  }

  for (int i = 0; i < VITREF_TABLE_SIZE; ++i)
    x->emission.null[i] = sample_float();

  for (int i = 0; i < VITREF_TABLE_SIZE; ++i)
    x->emission.background[i] = sample_float();

  for (int i = 0; i < VITREF_TABLE_SIZE * x->K; ++i)
    x->emission.match[i] = sample_float();
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
  x->BM = INFINITY;
  x->MM = INFINITY;
  x->MI = INFINITY;
  x->MD = INFINITY;
  x->IM = INFINITY;
  x->II = INFINITY;
  x->DM = INFINITY;
  x->DD = INFINITY;
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

static inline void core_state_init(struct core_state *x, int t, int k,
                                   int K)
{
  x[time_map(k, t, K)].M = INFINITY;
  x[time_map(k, t, K)].D = INFINITY;
  x[time_map(k, t, K)].I = INFINITY;
}

static inline void emission_init(struct emission *x, int K)
{
  for (int i = 0; i < VITREF_TABLE_SIZE; ++i)
  {
    x->null[i] = INFINITY;
    x->background[i] = INFINITY;
  }

  for (int i = 0; i < VITREF_TABLE_SIZE * K; ++i)
    x->match[i] = INFINITY;
}

static inline void core_advance(struct core_state *x, int K)
{
  for (int k = 0; k < K; ++k)
  {
    for (int t = VITREF_TIME_FRAME - 1; t > 0; --t)
    {
      x[time_map(k, t, K)].M = x[time_map(k, t - 1, K)].M;
      x[time_map(k, t, K)].D = x[time_map(k, t - 1, K)].D;
      x[time_map(k, t, K)].I = x[time_map(k, t - 1, K)].I;
    }
    core_state_init(x, 0, k, K);
  }
}

static inline void extr_advance(struct extr_state *x)
{
  for (int t = VITREF_TIME_FRAME - 1; t > 0; --t)
  {
    x[t].S = x[t - 1].S;
    x[t].N = x[t - 1].N;
    x[t].B = x[t - 1].B;

    x[t].J = x[t - 1].J;

    x[t].E = x[t - 1].E;
    x[t].C = x[t - 1].C;
    x[t].T = x[t - 1].T;
  }
  extr_state_init(x, 0);
}
