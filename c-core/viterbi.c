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

float viterbi_null(struct protein *x, struct imm_eseq const *eseq)
{
  int seq_size = imm_eseq_size(eseq);

  DECLARE_DP(S);
  DECLARE_DP(R);
  dp_fill(S, IMM_LPROB_ZERO);
  dp_fill(R, IMM_LPROB_ZERO);
  dp_set(S, 0, 0);

  DECLARE_INDEX(ix) = {0};
  DECLARE_TABLE(null) = {0};

  for (int r = 0; r < seq_size + 1; ++r)
  {
    index_setup(ix, eseq, r, false);
    table_setup(null, x->null.emission, ix, false);

    dp_set(R, 0, onto_R(S, R, x->null.RR, null));
    dp_advance(S);
    dp_advance(R);
    dp_set(S, 0, IMM_LPROB_ZERO);
  }
  return dp_get(R, 1);
}

INLINE void viterbi_on_range(struct protein const *x, struct viterbi *t,
                             struct imm_eseq const *eseq, int row_start,
                             int row_end, bool const safe, struct trellis *tr)
{
  int core_size = x->core_size;

  struct viterbi_xtrans const xt = viterbi_xtrans_init(x->xtrans);

  DECLARE_INDEX(ix) = {0};
  DECLARE_TABLE(null) = {0};
  DECLARE_TABLE(bg) = {0};
  DECLARE_TABLE(match) = {0};

  if (tr) trellis_seek_xnode(tr, row_start);
  if (tr) trellis_seek_node(tr, row_start, 0);
  for (int r = row_start; r < row_end; ++r)
  {
    if (tr) trellis_clear_xnode(tr);
    index_setup(ix, eseq, r, safe);
    table_setup(null, x->null.emission, ix, safe);
    table_setup(bg, x->bg.emission, ix, safe);

    dp_set(t->N, 0, onto_N(tr, t->S, t->N, xt.SN, xt.NN, null));
    dp_set(t->B, 0, onto_B(tr, t->S, t->N, xt.SB, xt.NB));

    dp_advance(t->S);
    dp_advance(t->N);

    float *Mk = coredp_rewind(t->dp, STATE_M);
    float *Ik = coredp_rewind(t->dp, STATE_I);
    float *Dk = coredp_rewind(t->dp, STATE_D);
    table_setup(match, x->nodes[0].emission, ix, safe);

    if (tr) trellis_clear_node(tr);
    // BM(0) -> M(0)
    dp_set(Mk, 0, onto_M0(tr, t->B, x->BMk[0], match));
    // M(0) -> E
    float Emax = dp_get(Mk, 0) + xt.ME + 0;
    // Skip transition into D0 state (does not exist)

    for (int k = 0; k + 1 < core_size; ++k)
    {
      int n = k + 1;
      float const MM = x->nodes[k].trans.MM;
      float const MI = x->nodes[k].trans.MI;
      float const MD = x->nodes[k].trans.MD;
      float const IM = x->nodes[k].trans.IM;
      float const II = x->nodes[k].trans.II;
      float const DM = x->nodes[k].trans.DM;
      float const DD = x->nodes[k].trans.DD;
      float const BM = x->BMk[n];
      table_setup(match, x->nodes[n].emission, ix, safe);

      // [M(k), I(k)] -> I(k)
      dp_set(Ik, 0, onto_I(tr, Mk, Ik, MI, II, bg));
      if (tr) trellis_next_node(tr);
      if (tr) trellis_clear_node(tr);

      // [BM(n), M(k), I(k), D(k)] -> M(n)
      float Mn = onto_M(tr, t->B, Mk, Ik, Dk, BM, MM, IM, DM, match);
      table_prefetch(x->nodes[k + 2].emission, ix);

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
      dp_set(t->E, 0, onto_E(tr, t->dp, xt.ME, xt.DE, core_size));
    else
      dp_set(t->E, 0, Emax);

    dp_set(t->J, 0, onto_J(tr, t->E, t->J, xt.EJ, xt.JJ, null));

    dp_set(t->B, 0, adjust_onto_B(tr, t->B, t->E, t->J, xt.EB, xt.JB));

    dp_advance(t->B);
    dp_advance(t->J);

    dp_set(t->C, 0, onto_C(tr, t->E, t->C, xt.EC, xt.CC, null));
    dp_set(t->T, 0, onto_T(tr, t->E, t->C, xt.ET, xt.CT));

    dp_advance(t->E);
    dp_advance(t->C);
    dp_advance(t->T);

    dp_set(t->S, 0, IMM_LPROB_ZERO);
    if (tr) trellis_next_xnode(tr);
  }
}

float viterbi_alt(struct viterbi *task, struct imm_eseq const *eseq,
                  bool const nopath)
{
  assert(imm_eseq_size(eseq) < INT_MAX);
  int seq_size = imm_eseq_size(eseq);

  dp_set(task->S, 0, 0);

  int row_start = 0;
  int row_mid =
      seq_size + 1 < (DCP_PAST_SIZE - 1) ? seq_size + 1 : (DCP_PAST_SIZE - 1);
  int row_end = seq_size + 1;

  struct protein const *x = task->protein;
  if (nopath)
  {
    viterbi_on_range(x, task, eseq, row_start, row_mid, false, NULL);
    viterbi_on_range(x, task, eseq, row_mid, row_end, true, NULL);
  }
  else
  {
    viterbi_on_range(x, task, eseq, row_start, row_mid, false, &task->trellis);
    viterbi_on_range(x, task, eseq, row_mid, row_end, true, &task->trellis);
  }

  return dp_get(task->T, 1);
}

int viterbi_alt_extract_path(struct viterbi *task, int seq_size)
{
  imm_path_reset(&task->path);
  return unzip_path(&task->trellis, seq_size, &task->path);
}

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

int viterbi_setup_path(struct viterbi *x)
{
  imm_path_reset(&x->path);
  int seq_size = imm_eseq_size(x->seq);
  return trellis_setup(&x->trellis, x->protein->core_size, seq_size);
}

void viterbi_cleanup(struct viterbi *x)
{
  trellis_cleanup(&x->trellis);
  coredp_cleanup(&x->dp);
  imm_path_cleanup(&x->path);
}
