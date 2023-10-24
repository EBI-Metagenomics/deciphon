#include "viterbi.h"
#include "array_size.h"
#include "array_size_field.h"
#include "compiler.h"
#include "defer_return.h"
#include "find_fmax.h"
#include "imm/imm.h"
#include "protein.h"
#include "protein_node.h"
#include "reduce_fmax.h"
#include "scan_thread.h"
#include "trellis.h"
#include "viterbi_emission.h"
#include "viterbi_onto.h"
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

#define NINF IMM_LPROB_ZERO
  float S[VITERBI_PAST_SIZE] = {NINF, NINF, NINF, NINF, NINF, NINF};
  float R[VITERBI_PAST_SIZE] = {NINF, NINF, NINF, NINF, NINF, NINF};
#undef NINF
  dp_set(S, 0, 0);

  int ix[VITERBI_PAST_SIZE - 1] ALIGNED = {0};
  float null[VITERBI_PAST_SIZE - 1] ALIGNED = {0};
  for (int r = 0; r < seq_size + 1; ++r)
  {
    emission_index(ix, eseq, r, false);
    emission_fetch(null, x->null.emission, ix, false);

    dp_set(R, 0, onto_R(S, R, x->null.RR, null));
    dp_advance(S);
    dp_advance(R);
    dp_set(S, 0, IMM_LPROB_ZERO);
  }
  return dp_of(R, 1);
}

INLINE void viterbi_on_range(struct protein *x, struct viterbi_task *task,
                             struct imm_eseq const *eseq, int row_start,
                             int row_end, bool const safe, struct trellis *tr)
{
  int core_size = x->core_size;

  struct viterbi_xtrans const xt = viterbi_xtrans(x->xtrans);
  float *restrict S = task->S;
  float *restrict N = task->N;
  float *restrict B = task->B;
  float *restrict J = task->J;
  float *restrict E = task->E;
  float *restrict C = task->C;
  float *restrict T = task->T;

  int ix[VITERBI_PAST_SIZE - 1] ALIGNED = {0};
  float null[VITERBI_PAST_SIZE - 1] ALIGNED = {0};
  float bg[VITERBI_PAST_SIZE - 1] ALIGNED = {0};
  float match[VITERBI_PAST_SIZE - 1] ALIGNED = {0};

  if (tr) trellis_seek_xnode(tr, row_start);
  if (tr) trellis_seek_node(tr, row_start, 0);
  for (int r = row_start; r < row_end; ++r)
  {
    if (tr) trellis_clear_xnode(tr);
    emission_index(ix, eseq, r, safe);
    emission_fetch(null, x->null.emission, ix, safe);
    emission_fetch(bg, x->bg.emission, ix, safe);

    dp_set(N, 0, onto_N(tr, S, N, xt.SN, xt.NN, null));
    dp_set(B, 0, onto_B(tr, S, N, xt.SB, xt.NB));

    dp_advance(S);
    dp_advance(N);

    float *Mk = dp_rewind(task->dp, STATE_M);
    float *Ik = dp_rewind(task->dp, STATE_I);
    float *Dk = dp_rewind(task->dp, STATE_D);
    emission_fetch(match, x->nodes[0].emission, ix, safe);

    if (tr) trellis_clear_node(tr);
    // BM(0) -> M(0)
    dp_set(Mk, 0, onto_M0(tr, B, x->BMk[0], match));
    // M(0) -> E
    float Emax = dp_of(Mk, 0) + xt.ME + 0;
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
      float Mn = onto_M(tr, B, Mk, Ik, Dk, BM, MM, IM, DM, match);
      emission_prefetc(x->nodes[k + 2].emission, ix);

      // [M(k), D(k)] -> D(n)
      float Dn = onto_D(tr, Mk, Dk, MD, DD);

      dp_advance(Mk);
      dp_advance(Ik);
      dp_advance(Dk);

      Mk = dp_next(Mk);
      dp_set(Mk, 0, Mn);

      Ik = dp_next(Ik);
      Dk = dp_next(Dk);
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
      dp_set(E, 0, onto_E(tr, task->dp, xt.ME, xt.DE, core_size));
    else
      dp_set(E, 0, Emax);

    dp_set(J, 0, onto_J(tr, E, J, xt.EJ, xt.JJ, null));

    dp_set(B, 0, adjust_onto_B(tr, B, E, J, xt.EB, xt.JB));

    dp_advance(B);
    dp_advance(J);

    dp_set(C, 0, onto_C(tr, E, C, xt.EC, xt.CC, null));
    dp_set(T, 0, onto_T(tr, E, C, xt.ET, xt.CT));

    dp_advance(E);
    dp_advance(C);
    dp_advance(T);

    dp_set(S, 0, IMM_LPROB_ZERO);
    if (tr) trellis_next_xnode(tr);
  }
}

static int unzip_path(struct trellis *x, int seq_size, struct imm_path *path);

int viterbi(struct protein *x, struct imm_eseq const *eseq,
            struct viterbi_task *task, bool const nopath)
{
  assert(imm_eseq_size(eseq) <= INT_MAX);
  int seq_size = (int)imm_eseq_size(eseq);
  int rc = viterbi_task_setup(task, x->core_size, seq_size, nopath);
  if (rc) return rc;

  dp_set(task->S, 0, 0);

  int row_start = 0;
  int row_mid = seq_size + 1 < 5 ? seq_size + 1 : 5;
  int row_end = seq_size + 1;

  if (nopath)
  {
    viterbi_on_range(x, task, eseq, row_start, row_mid, false, NULL);
    viterbi_on_range(x, task, eseq, row_mid, row_end, true, NULL);
    task->score = dp_of(task->T, 1);
  }
  else
  {
    viterbi_on_range(x, task, eseq, row_start, row_mid, false, &task->trellis);
    viterbi_on_range(x, task, eseq, row_mid, row_end, true, &task->trellis);
    task->score = dp_of(task->T, 1);
    imm_path_reset(&task->path);
    if (!imm_lprob_is_nan(task->score))
      rc = unzip_path(&task->trellis, seq_size, &task->path);
  }

  return rc;
}

static int unzip_path(struct trellis *x, int seq_size, struct imm_path *path)
{
  int state = state_make_end();
  assert(seq_size <= INT_MAX);
  int stage = (int)seq_size;
  trellis_seek_xnode(x, stage);

  while (!state_is_start(state) || stage)
  {
    int size = trellis_emission_size(x, state);
    if (imm_path_add(path, imm_step(state, size, 0))) return DCP_ENOMEM;
    state = trellis_previous_state(x, state);
    stage -= size;
    if (state_is_core(state))
      trellis_seek_node(x, stage, state_core_idx(state));
    else
      trellis_seek_xnode(x, stage);
  }
  if (imm_path_add(path, imm_step(state, 0, 0))) return DCP_ENOMEM;
  imm_path_reverse(path);
  return 0;
}
