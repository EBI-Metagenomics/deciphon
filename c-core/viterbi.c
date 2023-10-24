#include "imm/imm.h"
#include "protein.h"
#include "protein_node.h"
#include "viterbi_dp.h"
#include "viterbi_emission.h"
#include "viterbi_onto.h"
#include "viterbi_path.h"
#include "viterbi_task.h"
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

  DECLARE_EMISSION_INDEX(ix) = {0};
  DECLARE_EMISSION_TABLE(null) = {0};

  for (int r = 0; r < seq_size + 1; ++r)
  {
    emission_index(ix, eseq, r, false);
    emission_fetch(null, x->null.emission, ix, false);

    dp_set(R, 0, onto_R(S, R, x->null.RR, null));
    dp_advance(S);
    dp_advance(R);
    dp_set(S, 0, IMM_LPROB_ZERO);
  }
  return dp_get(R, 1);
}

INLINE void viterbi_on_range(struct protein const *x, struct viterbi_task *t,
                             struct imm_eseq const *eseq, int row_start,
                             int row_end, bool const safe, struct trellis *tr)
{
  int core_size = x->core_size;

  struct viterbi_xtrans const xt = viterbi_xtrans_init(x->xtrans);

  DECLARE_EMISSION_INDEX(ix) = {0};
  DECLARE_EMISSION_TABLE(null) = {0};
  DECLARE_EMISSION_TABLE(bg) = {0};
  DECLARE_EMISSION_TABLE(match) = {0};

  if (tr) trellis_seek_xnode(tr, row_start);
  if (tr) trellis_seek_node(tr, row_start, 0);
  for (int r = row_start; r < row_end; ++r)
  {
    if (tr) trellis_clear_xnode(tr);
    emission_index(ix, eseq, r, safe);
    emission_fetch(null, x->null.emission, ix, safe);
    emission_fetch(bg, x->bg.emission, ix, safe);

    dp_set(t->N, 0, onto_N(tr, t->S, t->N, xt.SN, xt.NN, null));
    dp_set(t->B, 0, onto_B(tr, t->S, t->N, xt.SB, xt.NB));

    dp_advance(t->S);
    dp_advance(t->N);

    float *Mk = dp_rewind(t->dp, STATE_M);
    float *Ik = dp_rewind(t->dp, STATE_I);
    float *Dk = dp_rewind(t->dp, STATE_D);
    emission_fetch(match, x->nodes[0].emission, ix, safe);

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
      emission_fetch(match, x->nodes[n].emission, ix, safe);

      // [M(k), I(k)] -> I(k)
      dp_set(Ik, 0, onto_I(tr, Mk, Ik, MI, II, bg));
      if (tr) trellis_next_node(tr);
      if (tr) trellis_clear_node(tr);

      // [BM(n), M(k), I(k), D(k)] -> M(n)
      float Mn = onto_M(tr, t->B, Mk, Ik, Dk, BM, MM, IM, DM, match);
      emission_prefetch(x->nodes[k + 2].emission, ix);

      // [M(k), D(k)] -> D(n)
      float Dn = onto_D(tr, Mk, Dk, MD, DD);

      dp_advance(Mk);
      dp_advance(Ik);
      dp_advance(Dk);

      Mk = dp_core_next(Mk);
      dp_set(Mk, 0, Mn);

      Ik = dp_core_next(Ik);
      Dk = dp_core_next(Dk);
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

float viterbi_alt(struct viterbi_task *task, struct imm_eseq const *eseq,
                  bool const nopath)
{
  assert(imm_eseq_size(eseq) < INT_MAX);
  int seq_size = imm_eseq_size(eseq);

  dp_set(task->S, 0, 0);

  int row_start = 0;
  int row_mid = seq_size + 1 < (PAST_SIZE - 1) ? seq_size + 1 : (PAST_SIZE - 1);
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

int viterbi_alt_extract_path(struct viterbi_task *task, int seq_size)
{
  imm_path_reset(&task->path);
  return unzip_path(&task->trellis, seq_size, &task->path);
}
