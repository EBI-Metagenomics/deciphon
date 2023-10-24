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

PURE int emission_index(struct imm_eseq const *x, int pos, int size,
                        bool const safe)
{
  return ((!safe && (pos) < 0) ? -1 : imm_eseq_get(x, pos, size, 1));
}

PURE float safe_get(float const x[restrict], int i, bool const safe)
{
  return (safe ? (x)[(i)] : (i) >= 0 ? (x)[(i)] : IMM_LPROB_ZERO);
}

PURE float const *unsafe_get(float const x[restrict], int i) { return x + i; }

INLINE void fetch_indices(int x[restrict], struct imm_eseq const *eseq, int row,
                          bool const safe)
{
  x[0] = emission_index(eseq, row - 1, 1, safe);
  x[1] = emission_index(eseq, row - 2, 2, safe);
  x[2] = emission_index(eseq, row - 3, 3, safe);
  x[3] = emission_index(eseq, row - 4, 4, safe);
  x[4] = emission_index(eseq, row - 5, 5, safe);
}

INLINE void fetch_emission(float x[restrict], float emission[restrict],
                           int const ix[restrict], bool const safe)
{
  x[0] = safe_get(emission, ix[nchars(1)], safe);
  x[1] = safe_get(emission, ix[nchars(2)], safe);
  x[2] = safe_get(emission, ix[nchars(3)], safe);
  x[3] = safe_get(emission, ix[nchars(4)], safe);
  x[4] = safe_get(emission, ix[nchars(5)], safe);
}

INLINE void prefetch_emission(float emission[restrict], int const ix[restrict])
{
  PREFETCH(unsafe_get(emission, ix[nchars(1)]), 0, 1);
  PREFETCH(unsafe_get(emission, ix[nchars(2)]), 0, 1);
  PREFETCH(unsafe_get(emission, ix[nchars(3)]), 0, 1);
  PREFETCH(unsafe_get(emission, ix[nchars(4)]), 0, 1);
  PREFETCH(unsafe_get(emission, ix[nchars(5)]), 0, 1);
}

INLINE void make_future(float x[])
{
  memmove(&x[lukbak(1)], &x[lukbak(0)],
          sizeof(float) * (VITERBI_PAST_SIZE - 1));
}

float viterbi_null(struct protein *x, struct imm_eseq const *eseq)
{
  int seq_size = (int)imm_eseq_size(eseq);

#define NINF IMM_LPROB_ZERO
  float S[VITERBI_PAST_SIZE] = {NINF, NINF, NINF, NINF, NINF, NINF};
  float R[VITERBI_PAST_SIZE] = {NINF, NINF, NINF, NINF, NINF, NINF};
#undef NINF
  S[lukbak(0)] = 0;

  int ix[VITERBI_PAST_SIZE - 1] ALIGNED = {0};
  float null[VITERBI_PAST_SIZE - 1] ALIGNED = {0};
  for (int r = 0; r < seq_size + 1; ++r)
  {
    fetch_indices(ix, eseq, r, false);
    fetch_emission(null, x->null.emission, ix, false);

    R[lukbak(0)] = onto_R(S, R, x->null.RR, null);
    make_future(S);
    make_future(R);
    S[lukbak(0)] = IMM_LPROB_ZERO;
  }
  return R[lukbak(1)];
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
    fetch_indices(ix, eseq, r, safe);
    fetch_emission(null, x->null.emission, ix, safe);
    fetch_emission(bg, x->bg.emission, ix, safe);

    N[lukbak(0)] = onto_N(tr, S, N, xt.SN, xt.NN, null);
    B[lukbak(0)] = onto_B(tr, S, N, xt.SB, xt.NB);

    make_future(S);
    make_future(N);

    float *Mk = dp_rewind(task->dp, STATE_M);
    float *Ik = dp_rewind(task->dp, STATE_I);
    float *Dk = dp_rewind(task->dp, STATE_D);
    fetch_emission(match, x->nodes[0].emission, ix, safe);

    if (tr) trellis_clear_node(tr);
    // BM(0) -> M(0)
    Mk[lukbak(0)] = onto_M0(tr, B, x->BMk[0], match);
    // M(0) -> E
    float Emax = Mk[lukbak(0)] + xt.ME + 0;
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
      fetch_emission(match, x->nodes[n].emission, ix, safe);

      // [M(k), I(k)] -> I(k)
      Ik[lukbak(0)] = onto_I(tr, Mk, Ik, MI, II, bg);
      if (tr) trellis_next_node(tr);
      if (tr) trellis_clear_node(tr);

      // [BM(n), M(k), I(k), D(k)] -> M(n)
      float Mn = onto_M(tr, B, Mk, Ik, Dk, BM, MM, IM, DM, match);
      prefetch_emission(x->nodes[k + 2].emission, ix);

      // [M(k), D(k)] -> D(n)
      float Dn = onto_D(tr, Mk, Dk, MD, DD);

      make_future(Mk);
      make_future(Ik);
      make_future(Dk);

      Mk = dp_next(Mk);
      Mk[lukbak(0)] = Mn;

      Ik = dp_next(Ik);
      Dk = dp_next(Dk);
      Dk[lukbak(0)] = Dn;

      Emax = float_maximum(Emax, Mn + xt.ME + 0);
      Emax = float_maximum(Emax, Dn + xt.DE + 0);
    }
    // Skip transition into Ik1 state (does not exist)
    if (tr) trellis_next_node(tr);
    make_future(Mk);
    make_future(Ik);
    make_future(Dk);

    if (tr)
      E[lukbak(0)] = onto_E(tr, task->dp, xt.ME, xt.DE, core_size);
    else
      E[lukbak(0)] = Emax;

    J[lukbak(0)] = onto_J(tr, E, J, xt.EJ, xt.JJ, null);

    B[lukbak(0)] = adjust_onto_B(tr, B, E, J, xt.EB, xt.JB);

    make_future(B);
    make_future(J);

    C[lukbak(0)] = onto_C(tr, E, C, xt.EC, xt.CC, null);
    T[lukbak(0)] = onto_T(tr, E, C, xt.ET, xt.CT);

    make_future(E);
    make_future(C);
    make_future(T);

    S[lukbak(0)] = IMM_LPROB_ZERO;
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

  task->S[lukbak(0)] = 0;

  int row_start = 0;
  int row_mid = seq_size + 1 < 5 ? seq_size + 1 : 5;
  int row_end = seq_size + 1;

  if (nopath)
  {
    viterbi_on_range(x, task, eseq, row_start, row_mid, false, NULL);
    viterbi_on_range(x, task, eseq, row_mid, row_end, true, NULL);
    task->score = task->T[lukbak(1)];
  }
  else
  {
    viterbi_on_range(x, task, eseq, row_start, row_mid, false, &task->trellis);
    viterbi_on_range(x, task, eseq, row_mid, row_end, true, &task->trellis);
    task->score = task->T[lukbak(1)];
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

void viterbi_dump(struct protein *x, FILE *fp)
{
  int core_size = x->core_size;

  float const mute_emission = IMM_LPROB_ONE;
  float const emis_B = mute_emission;
  float const emis_D = mute_emission;
  float const emis_E = mute_emission;
  float const emis_T = mute_emission;

  char const *f32f = imm_fmt_get_f32();

  fprintf(fp, "B: ");
  fprintf(fp, f32f, emis_B);
  fputc('\n', fp);

  fprintf(fp, "D: ");
  fprintf(fp, f32f, emis_D);
  fputc('\n', fp);

  fprintf(fp, "E: ");
  fprintf(fp, f32f, emis_E);
  fputc('\n', fp);

  fprintf(fp, "T: ");
  fprintf(fp, f32f, emis_T);
  fputc('\n', fp);

  float const *restrict null_emission = x->null.emission;
  float const *restrict background_emission = x->bg.emission;
  float const *restrict emis_I = background_emission;
  float const *restrict emis_N = null_emission;
  float const *restrict emis_J = null_emission;
  float const *restrict emis_C = null_emission;
  size_t bg_size = array_size_field(struct protein_background, emission);

  fprintf(fp, "I*: ");
  imm_dump_array_f32(bg_size, emis_I, fp);
  fputc('\n', fp);

  fprintf(fp, "N: [");
  imm_dump_array_f32(bg_size, emis_N, fp);
  fprintf(fp, "]\n");

  fprintf(fp, "J: [");
  imm_dump_array_f32(bg_size, emis_J, fp);
  fprintf(fp, "]\n");

  fprintf(fp, "C: [");
  imm_dump_array_f32(bg_size, emis_C, fp);
  fprintf(fp, "]\n");

  for (int k = 0; k < core_size; ++k)
  {
    float const *match_emission = x->nodes[k].emission;

    fprintf(fp, "M%d: ", k + 1);
    size_t n = PROTEIN_NODE_SIZE;
    imm_dump_array_f32(n, match_emission, fp);
    fputc('\n', fp);
  }
}

void viterbi_dump_dot(struct protein *x, FILE *fp)
{
  char const *f32f = imm_fmt_get_f32();

  struct viterbi_xtrans const xtrans = viterbi_xtrans(x->xtrans);

  fprintf(fp, "S -> B [label=");
  fprintf(fp, f32f, xtrans.SB);
  fprintf(fp, "];\n");

  fprintf(fp, "S -> N [label=");
  fprintf(fp, f32f, xtrans.SN);
  fprintf(fp, "];\n");

  fprintf(fp, "N -> N [label=");
  fprintf(fp, f32f, xtrans.NN);
  fprintf(fp, "];\n");

  fprintf(fp, "N -> B [label=");
  fprintf(fp, f32f, xtrans.NB);
  fprintf(fp, "];\n");

  fprintf(fp, "E -> T [label=");
  fprintf(fp, f32f, xtrans.ET);
  fprintf(fp, "];\n");

  fprintf(fp, "E -> C [label=");
  fprintf(fp, f32f, xtrans.EC);
  fprintf(fp, "];\n");

  fprintf(fp, "C -> C [label=");
  fprintf(fp, f32f, xtrans.CC);
  fprintf(fp, "];\n");

  fprintf(fp, "C -> T [label=");
  fprintf(fp, f32f, xtrans.CT);
  fprintf(fp, "];\n");

  fprintf(fp, "E -> B [label=");
  fprintf(fp, f32f, xtrans.EB);
  fprintf(fp, "];\n");

  fprintf(fp, "E -> J [label=");
  fprintf(fp, f32f, xtrans.EJ);
  fprintf(fp, "];\n");

  fprintf(fp, "J -> J [label=");
  fprintf(fp, f32f, xtrans.JJ);
  fprintf(fp, "];\n");

  fprintf(fp, "J -> B [label=");
  fprintf(fp, f32f, xtrans.JB);
  fprintf(fp, "];\n");

  int core_size = x->core_size;
  for (int k = 0; k + 1 < core_size; ++k)
  {
    struct trans const *restrict trans = &x->nodes[k].trans;
    int i0 = k + 1;
    int i1 = k + 2;
    fprintf(fp, "D%d -> D%d [label=", i0, i1);
    fprintf(fp, f32f, trans->DD);
    fprintf(fp, "];\n");

    fprintf(fp, "D%d -> M%d [label=", i0, i1);
    fprintf(fp, f32f, trans->DM);
    fprintf(fp, "];\n");

    fprintf(fp, "I%d -> I%d [label=", i0, i0);
    fprintf(fp, f32f, trans->II);
    fprintf(fp, "];\n");

    fprintf(fp, "I%d -> M%d [label=", i0, i1);
    fprintf(fp, f32f, trans->IM);
    fprintf(fp, "];\n");

    fprintf(fp, "M%d -> I%d [label=", i0, i0);
    fprintf(fp, f32f, trans->MI);
    fprintf(fp, "];\n");

    fprintf(fp, "M%d -> M%d [label=", i0, i1);
    fprintf(fp, f32f, trans->MM);
    fprintf(fp, "];\n");

    fprintf(fp, "M%d -> D%d [label=", i0, i1);
    fprintf(fp, f32f, trans->MD);
    fprintf(fp, "];\n");

    fprintf(fp, "M%d -> E [label=", i0);
    fprintf(fp, f32f, xtrans.ME);
    fprintf(fp, "];\n");

    fprintf(fp, "D%d -> E [label=", i0);
    fprintf(fp, f32f, xtrans.DE);
    fprintf(fp, "];\n");
  }

  fprintf(fp, "M%d -> E [label=", core_size);
  fprintf(fp, f32f, xtrans.ME);
  fprintf(fp, "];\n");

  fprintf(fp, "D%d -> E [label=", core_size);
  fprintf(fp, f32f, xtrans.DE);
  fprintf(fp, "];\n");

  float const *restrict trans_BM = x->BMk;
  for (int k = 0; k < core_size; ++k)
  {
    fprintf(fp, "B -> M%d [label=", k + 1);
    fprintf(fp, f32f, trans_BM[k]);
    fprintf(fp, "];\n");
  }
}
