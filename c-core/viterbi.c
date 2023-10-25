#include "imm/imm.h"
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
  x->path = imm_path();
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
  imm_path_cleanup(&x->path);
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

      // [BM(n), M(k), I(k), D(k)] -> M(n)
      float Mn = onto_M(tr, x->B, Mk, Ik, Dk, BM, MM, IM, DM, match);
      table_prefetch(x->protein->nodes[k + 2].emission, ix);

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

      Emax = float_maximum(Emax, Mn + xt.ME + 0);
      Emax = float_maximum(Emax, Dn + xt.DE + 0);
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

float viterbi_alt_loglik(struct viterbi *x)
{
  assert(imm_eseq_size(eseq) < INT_MAX);
  int seq_size = imm_eseq_size(x->seq);
  int end = seq_size + 1;

  dp_set(x->S, 0, 0);
  alternative(x, 0, row_mid(end), false, NULL);
  alternative(x, row_mid(end), end, true, NULL);
  return dp_get(x->T, 1);
}

int viterbi_alt_path(struct viterbi *x, float *loglik)
{
  assert(imm_eseq_size(eseq) < INT_MAX);
  int seq_size = imm_eseq_size(x->seq);
  int end = seq_size + 1;

  int rc = trellis_setup(&x->trellis, x->protein->core_size, seq_size);
  if (rc) return rc;

  dp_set(x->S, 0, 0);
  alternative(x, 0, row_mid(end), false, &x->trellis);
  alternative(x, row_mid(end), end, true, &x->trellis);
  if (loglik) *loglik = dp_get(x->T, 1);

  imm_path_reset(&x->path);
  return unzip_path(&x->trellis, seq_size, &x->path);
}
