#include "tictoc.h"
#include "imm/lprob.h"
#include "vitfast.h"
#include "imm/path.h"
#include "protein.h"
#include "protein_node.h"
#include "viterbi_dp.h"
#include "viterbi_index.h"
#include "viterbi_onto.h"
#include "viterbi_path.h"
#include "viterbi_struct.h"
#include "viterbi_table.h"
#include "viterbi_xtrans.h"
#include <stdlib.h>
#include <string.h>

// Let m be the core size.
// We evaluate the HMM in the following order:
//
//          -> S
//   (S, N) -> N
//   (S, N) -> B' (it will be adjusted later on)
//
//   B0 -> M0
//   M0 -> E'     (it will be adjusted later on)
//   For each k in 0, 1, m-1:
//     Let n = k + 1.
//     (Mk, Ik)             -> Ik
//     (B', Mk, Ik, Dk)     -> Mn
//     (Mk, Dk)             -> Dn
//     (E', Mn, Dn)         -> E'
//
//   E'         -> E
//   (E , J)    -> J
//   (B', E, J) -> B
//   (E , C)    -> C
//   (E , C)    -> T

void viterbi_init(struct viterbi *x)
{
  x->protein = NULL;
  coredp_init(&x->dp);
  trellis_init(&x->trellis);
}

int viterbi_setup(struct viterbi *x, struct protein const *protein,
                  struct imm_eseq const *eseq)
{
  x->protein = protein;
  x->seq = eseq;

  dp_fill(x->S, IMM_LPROB_ZERO);
  dp_fill(x->N, IMM_LPROB_ZERO);
  dp_fill(x->B, IMM_LPROB_ZERO);
  dp_fill(x->J, IMM_LPROB_ZERO);
  dp_fill(x->E, IMM_LPROB_ZERO);
  dp_fill(x->C, IMM_LPROB_ZERO);
  dp_fill(x->T, IMM_LPROB_ZERO);

  return coredp_setup(&x->dp, x->protein->core_size);
}

void viterbi_cleanup(struct viterbi *x)
{
  trellis_cleanup(&x->trellis);
  coredp_cleanup(&x->dp);
}

float viterbi_null_loglik(struct viterbi *x)
{
  int seq_size = imm_eseq_size(x->seq);

  dp_fill(x->S, IMM_LPROB_ZERO);
  dp_fill(x->R, IMM_LPROB_ZERO);
  dp_set(x->S, 0, 0);

  DECLARE_INDEX(ix) = {0};
  DECLARE_TABLE(null) = {0};

  for (int r = 0; r < seq_size + 1; ++r)
  {
    index_setup(ix, x->seq, r, false);
    table_setup(null, x->protein->null.emission, ix, false);

    dp_set(x->R, 0, onto_R(x->S, x->R, x->protein->null.RR, null));
    dp_advance(x->S);
    dp_advance(x->R);
    dp_set(x->S, 0, IMM_LPROB_ZERO);
  }
  return dp_get(x->R, 1);
}

INLINE void alternative(struct viterbi *x, int row_start, int row_end,
                        bool const safe, struct trellis *tr)
{
  int core_size = x->protein->core_size;

  struct viterbi_xtrans const xt = viterbi_xtrans_init(x->protein->xtrans);

  DECLARE_INDEX(ix) = {0};
  DECLARE_TABLE(null) = {0};
  DECLARE_TABLE(bg) = {0};
  DECLARE_TABLE(match) = {0};

  if (tr) trellis_seek_xnode(tr, row_start);
  if (tr) trellis_seek_node(tr, row_start, 0);
  for (int r = row_start; r < row_end; ++r)
  {
    if (tr) trellis_clear_xnode(tr);
    index_setup(ix, x->seq, r, safe);
    table_setup(null, x->protein->null.emission, ix, safe);
    table_setup(bg, x->protein->bg.emission, ix, safe);

    dp_set(x->N, 0, onto_N(tr, x->S, x->N, xt.SN, xt.NN, null));
    dp_set(x->B, 0, onto_B(tr, x->S, x->N, xt.SB, xt.NB));

    dp_advance(x->S);
    dp_advance(x->N);

    float *Mk = coredp_rewind(x->dp, STATE_M);
    float *Ik = coredp_rewind(x->dp, STATE_I);
    float *Dk = coredp_rewind(x->dp, STATE_D);
    table_setup(match, x->protein->nodes[0].emission, ix, safe);

    if (tr) trellis_clear_node(tr);
    // BM(0) -> M(0)
    dp_set(Mk, 0, onto_M0(tr, x->B, x->protein->BMk[0], match));
    // M(0) -> E
    float Emax = dp_get(Mk, 0) + xt.ME + 0;
    // Skip transition into D0 state (does not exist)

    for (int k = 0; k + 1 < core_size; ++k)
    {
      int n = k + 1;
      float const MM = x->protein->nodes[k].trans.MM;
      float const MI = x->protein->nodes[k].trans.MI;
      float const MD = x->protein->nodes[k].trans.MD;
      float const IM = x->protein->nodes[k].trans.IM;
      float const II = x->protein->nodes[k].trans.II;
      float const DM = x->protein->nodes[k].trans.DM;
      float const DD = x->protein->nodes[k].trans.DD;
      float const BM = x->protein->BMk[n];
      table_setup(match, x->protein->nodes[n].emission, ix, safe);

      // [M(k), I(k)] -> I(k)
      dp_set(Ik, 0, onto_I(tr, Mk, Ik, MI, II, bg));
      if (tr) trellis_next_node(tr);
      if (tr) trellis_clear_node(tr);

      table_prefetch(x->protein->nodes[n + 1].emission, ix);
      // [BM(n), M(k), I(k), D(k)] -> M(n)
      float Mn = onto_M(tr, x->B, Mk, Ik, Dk, BM, MM, IM, DM, match);

      // [M(k), D(k)] -> D(n)
      float Dn = onto_D(tr, Mk, Dk, MD, DD);

      dp_advance(Mk);
      dp_advance(Ik);
      dp_advance(Dk);

      Mk = coredp_next(Mk);
      dp_set(Mk, 0, Mn);

      Ik = coredp_next(Ik);
      Dk = coredp_next(Dk);
      dp_set(Dk, 0, Dn);

      Emax = maximum(Emax, Mn + xt.ME + 0);
      Emax = maximum(Emax, Dn + xt.DE + 0);
    }
    // Skip transition into Ik1 state (does not exist)
    if (tr) trellis_next_node(tr);
    dp_advance(Mk);
    dp_advance(Ik);
    dp_advance(Dk);

    if (tr)
      dp_set(x->E, 0, onto_E(tr, x->dp, xt.ME, xt.DE, core_size));
    else
      dp_set(x->E, 0, Emax);

    dp_set(x->J, 0, onto_J(tr, x->E, x->J, xt.EJ, xt.JJ, null));

    dp_set(x->B, 0, adjust_onto_B(tr, x->B, x->E, x->J, xt.EB, xt.JB));

    dp_advance(x->B);
    dp_advance(x->J);

    dp_set(x->C, 0, onto_C(tr, x->E, x->C, xt.EC, xt.CC, null));
    dp_set(x->T, 0, onto_T(tr, x->E, x->C, xt.ET, xt.CT));

    dp_advance(x->E);
    dp_advance(x->C);
    dp_advance(x->T);

    dp_set(x->S, 0, IMM_LPROB_ZERO);
    if (tr) trellis_next_xnode(tr);
  }
}

static inline int row_mid(int end)
{
  return end < (DCP_PAST_SIZE - 1) ? end : (DCP_PAST_SIZE - 1);
}

static int code_fn(int pos, int len, void *arg)
{
  struct imm_eseq const *seq = arg;
  return imm_eseq_get(seq, pos, len, 1);
}

float viterbi_alt_loglik(struct viterbi *x)
{
  assert(imm_eseq_size(x->seq) < INT_MAX);
  int seq_size = imm_eseq_size(x->seq);
  int end = seq_size + 1;

  dp_set(x->S, 0, 0);
  tic();
  alternative(x, 0, row_mid(end), false, NULL);
  alternative(x, row_mid(end), end, true, NULL);
  toc("slow");
  float slow = dp_get(x->T, 1);

  struct vitfast *vit = vitfast_new();
  int K = x->protein->core_size;
  vitfast_setup(vit, K);

  struct viterbi_xtrans const xt = viterbi_xtrans_init(x->protein->xtrans);
  vitfast_set_extr_trans(vit, EXTR_TRANS_SN, -xt.SN);
  vitfast_set_extr_trans(vit, EXTR_TRANS_NN, -xt.NN);
  vitfast_set_extr_trans(vit, EXTR_TRANS_SB, -xt.SB);
  vitfast_set_extr_trans(vit, EXTR_TRANS_NB, -xt.NB);
  vitfast_set_extr_trans(vit, EXTR_TRANS_EB, -xt.EB);
  vitfast_set_extr_trans(vit, EXTR_TRANS_JB, -xt.JB);
  vitfast_set_extr_trans(vit, EXTR_TRANS_EJ, -xt.EJ);
  vitfast_set_extr_trans(vit, EXTR_TRANS_JJ, -xt.JJ);
  vitfast_set_extr_trans(vit, EXTR_TRANS_EC, -xt.EC);
  vitfast_set_extr_trans(vit, EXTR_TRANS_CC, -xt.CC);
  vitfast_set_extr_trans(vit, EXTR_TRANS_ET, -xt.ET);
  vitfast_set_extr_trans(vit, EXTR_TRANS_CT, -xt.CT);

  for (int k = 0; k < K; ++k)
  {
    struct protein const *p = x->protein;
    vitfast_set_core_trans(vit, CORE_TRANS_BM, -p->BMk[k], k);
  }

  vitfast_set_core_trans(vit, CORE_TRANS_MM, INFINITY, 0);
  vitfast_set_core_trans(vit, CORE_TRANS_MD, INFINITY, 0);
  vitfast_set_core_trans(vit, CORE_TRANS_IM, INFINITY, 0);
  vitfast_set_core_trans(vit, CORE_TRANS_DM, INFINITY, 0);
  vitfast_set_core_trans(vit, CORE_TRANS_DD, INFINITY, 0);
  for (int k = 0; k < K - 1; ++k)
  {
    struct protein const *p = x->protein;
    vitfast_set_core_trans(vit, CORE_TRANS_MM, -p->nodes[k].trans.MM, k + 1);
    vitfast_set_core_trans(vit, CORE_TRANS_MI, -p->nodes[k].trans.MI, k + 0);
    vitfast_set_core_trans(vit, CORE_TRANS_MD, -p->nodes[k].trans.MD, k + 1);
    vitfast_set_core_trans(vit, CORE_TRANS_IM, -p->nodes[k].trans.IM, k + 1);
    vitfast_set_core_trans(vit, CORE_TRANS_II, -p->nodes[k].trans.II, k + 0);
    vitfast_set_core_trans(vit, CORE_TRANS_DM, -p->nodes[k].trans.DM, k + 1);
    vitfast_set_core_trans(vit, CORE_TRANS_DD, -p->nodes[k].trans.DD, k + 1);
  }
  vitfast_set_core_trans(vit, CORE_TRANS_MI, INFINITY, K - 1);
  vitfast_set_core_trans(vit, CORE_TRANS_II, INFINITY, K - 1);

  for (size_t i = 0; i < VITFAST_TABLE_SIZE; ++i)
  {
    vitfast_set_null(vit, -x->protein->null.emission[i], i);
    vitfast_set_background(vit, -x->protein->bg.emission[i], i);

    for (int k = 0; k < K; ++k)
      vitfast_set_match(vit, -x->protein->nodes[k].emission[i], k, i);
  }

  tic();
  float fast = -vitfast_cost(vit, seq_size, code_fn, (void *)x->seq);
  toc("fast");
  if (fabsf(slow - fast) > 1e-7) {printf("%g %g: %g\n", slow, fast, slow-fast);exit(1);}

  vitfast_del(vit);

  return dp_get(x->T, 1);
}

int viterbi_alt_path(struct viterbi *x, struct imm_path *path, float *loglik)
{
  assert(imm_eseq_size(x->seq) < INT_MAX);
  int seq_size = imm_eseq_size(x->seq);
  int end = seq_size + 1;

  int rc = trellis_setup(&x->trellis, x->protein->core_size, seq_size);
  if (rc) return rc;

  dp_set(x->S, 0, 0);
  alternative(x, 0, row_mid(end), false, &x->trellis);
  alternative(x, row_mid(end), end, true, &x->trellis);
  if (loglik) *loglik = dp_get(x->T, 1);

  imm_path_reset(path);
  return unzip_path(&x->trellis, seq_size, path);
}
