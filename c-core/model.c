#include "model.h"
#include "deciphon.h"
#include "defer_return.h"
#include "entry_dist.h"
#include "error.h"
#include "imm_amino_lprob.h"
#include "imm_codon_lprob.h"
#include "imm_gencode.h"
#include "imm_hmm.h"
#include "imm_lprob.h"
#include "imm_nuclt_code.h"
#include "model.h"
#include "nuclt_dist.h"
#include "state.h"
#include "xrealloc.h"
#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define LOGN2 -1.3862943611198906f /* log(1./4.) */
#define LOG1 IMM_LPROB_ONE

static float const uniform_lprobs[IMM_NUCLT_SIZE] = {LOGN2, LOGN2, LOGN2,
                                                     LOGN2};

/* Compute log(1 - p) given log(p). */
static inline float log1_p(float logp) { return log1pf(-expf(logp)); }

int add_xnodes(struct model *);
void init_xnodes(struct model *);

void calculate_occupancy(struct model *);

bool have_called_setup(struct model *);
bool have_finished_add(struct model const *);

void init_delete(struct imm_mute_state *, struct model *);
void init_insert(struct imm_frame_state *, float epsilon,
                 struct nuclt_dist const *, int node_idx);
void init_match(struct imm_frame_state *, struct model *, struct nuclt_dist *);

int init_null_xtrans(struct imm_hmm *, struct model_xnode_null *);
int init_alt_xtrans(struct imm_hmm *, struct model_xnode_alt *);

struct imm_nuclt_lprob nuclt_lprob(struct imm_gencode const *,
                                   struct imm_codon_lprob const *);
struct imm_codon_lprob codon_lprob(struct imm_gencode const *,
                                   struct imm_amino const *,
                                   struct imm_amino_lprob const *,
                                   struct imm_nuclt const *);

void setup_nuclt_dist(struct imm_gencode const *, struct nuclt_dist *,
                      struct imm_amino const *, struct imm_nuclt const *,
                      float const[IMM_AMINO_SIZE]);

int setup_entry_trans(struct model *);
int setup_exit_trans(struct model *);
int setup_transitions(struct model *);

int model_add_node(struct model *x, float const lprobs[], char consensus)
{
  if (!have_called_setup(x)) return error(DCP_EFUNCUSE);

  if (x->alt.node_idx == x->core_size) return error(DCP_ELARGEMODEL);

  x->consensus[x->alt.node_idx] = consensus;

  float lodds[IMM_AMINO_SIZE];
  for (int i = 0; i < IMM_AMINO_SIZE; ++i)
    lodds[i] = lprobs[i] - x->null.lprobs[i];

  struct model_node *n = x->alt.nodes + x->alt.node_idx;

  setup_nuclt_dist(x->params.gencode, &n->match.nucltd, x->params.amino,
                   x->params.code->nuclt, lodds);

  init_match(&n->M, x, &n->match.nucltd);
  if (imm_hmm_add_state(x->alt.hmm, &n->M.super)) return error(DCP_EADDSTATE);

  init_insert(&n->I, x->params.epsilon, &x->alt.insert.nucltd, x->alt.node_idx);
  if (x->alt.node_idx == 0)
    init_insert(&x->background.state, x->params.epsilon,
                &x->background.nuclt_dist, 0);
  if (imm_hmm_add_state(x->alt.hmm, &n->I.super)) return error(DCP_EADDSTATE);

  init_delete(&n->D, x);
  if (imm_hmm_add_state(x->alt.hmm, &n->D.super)) return error(DCP_EADDSTATE);

  x->alt.node_idx++;

  if (have_finished_add(x)) setup_transitions(x);

  return 0;
}

int model_add_trans(struct model *x, struct trans trans)
{
  if (!have_called_setup(x)) return error(DCP_EFUNCUSE);

  if (x->alt.trans_idx == x->core_size + 1) return error(DCP_EMANYTRANS);

  x->alt.trans[x->alt.trans_idx++] = trans;
  if (have_finished_add(x)) setup_transitions(x);
  return 0;
}

void model_cleanup(struct model *x)
{
  free(x->alt.nodes);
  free(x->alt.locc);
  free(x->alt.trans);
  free(x->BMk);
  imm_hmm_del(x->alt.hmm);
  imm_hmm_del(x->null.hmm);
  x->alt.nodes = NULL;
  x->alt.locc = NULL;
  x->alt.trans = NULL;
  x->BMk = NULL;
  x->alt.hmm = NULL;
  x->null.hmm = NULL;
}

int model_init(struct model *x, struct model_params params,
               float const null_lprobs[])
{
  x->params = params;
  x->core_size = 0;
  x->has_ga = false;
  x->consensus[0] = '\0';
  x->BMk = NULL;
  x->alt.hmm = NULL;
  x->null.hmm = NULL;
  struct imm_nuclt const *nuclt = x->params.code->nuclt;

  memcpy(x->null.lprobs, null_lprobs, sizeof *null_lprobs * IMM_AMINO_SIZE);

  if (!(x->null.hmm = imm_hmm_new(&x->params.code->super)))
    return error(DCP_ENOMEM);

  setup_nuclt_dist(params.gencode, &x->null.nuclt_dist, params.amino, nuclt,
                   null_lprobs);

  if (!(x->alt.hmm = imm_hmm_new(&params.code->super)))
  {
    imm_hmm_del(x->null.hmm);
    return error(DCP_ENOMEM);
  }

  float const lodds[IMM_AMINO_SIZE] = {0};
  setup_nuclt_dist(params.gencode, &x->alt.insert.nucltd, params.amino, nuclt,
                   lodds);
  setup_nuclt_dist(params.gencode, &x->background.nuclt_dist, params.amino,
                   nuclt, lodds);

  init_xnodes(x);

  x->alt.node_idx = UINT_MAX;
  x->alt.nodes = NULL;
  x->alt.locc = NULL;
  x->alt.trans_idx = UINT_MAX;
  x->alt.trans = NULL;
  xtrans_init(&x->xtrans);
  return 0;
}

static void model_reset(struct model *x)
{
  imm_hmm_reset(x->null.hmm);
  imm_hmm_reset(x->alt.hmm);

  imm_state_detach(&x->xnode.null.F.super);
  imm_state_detach(&x->xnode.null.R.super);
  imm_state_detach(&x->xnode.null.G.super);

  imm_state_detach(&x->xnode.alt.S.super);
  imm_state_detach(&x->xnode.alt.N.super);
  imm_state_detach(&x->xnode.alt.B.super);
  imm_state_detach(&x->xnode.alt.E.super);
  imm_state_detach(&x->xnode.alt.J.super);
  imm_state_detach(&x->xnode.alt.C.super);
  imm_state_detach(&x->xnode.alt.T.super);
}

int model_setup(struct model *x, int core_size)
{
  int rc = 0;
  if (core_size == 0) return error(DCP_EZEROMODEL);

  if (core_size > MODEL_MAX) return error(DCP_ELARGEMODEL);

  x->core_size = core_size;
  x->consensus[core_size] = '\0';
  int n = x->core_size;
  x->alt.node_idx = 0;

  x->BMk = xrealloc(x->BMk, n * sizeof(*x->BMk));
  if (!x->BMk && n > 0) defer_return(error(DCP_ENOMEM));

  x->alt.nodes = xrealloc(x->alt.nodes, n * sizeof(*x->alt.nodes));
  if (!x->alt.nodes && n > 0) defer_return(error(DCP_ENOMEM));

  if (x->params.entry_dist == ENTRY_DIST_OCCUPANCY)
  {
    x->alt.locc = xrealloc(x->alt.locc, n * sizeof(*x->alt.locc));
    if (!x->alt.locc && n > 0) defer_return(error(DCP_ENOMEM));
  }
  x->alt.trans_idx = 0;
  x->alt.trans = xrealloc(x->alt.trans, (n + 1) * sizeof(*x->alt.trans));
  if (!x->alt.trans) defer_return(error(DCP_ENOMEM));

  model_reset(x);
  return error(add_xnodes(x));

defer:
  free(x->alt.nodes);
  free(x->alt.locc);
  free(x->alt.trans);
  x->alt.nodes = NULL;
  x->alt.locc = NULL;
  x->alt.trans = NULL;
  return rc;
}

char *state_name_wrap(int id, char *name)
{
  return error(state_name(id, name)) ? NULL : name;
}

void model_write_dot(struct model const *x, FILE *fp)
{
  imm_hmm_dump(x->alt.hmm, state_name_wrap, fp);
}

int add_xnodes(struct model *x)
{
  struct model_xnode *n = &x->xnode;

  if (imm_hmm_add_state(x->null.hmm, &n->null.F.super)) return error(DCP_EADDSTATE);
  if (imm_hmm_add_state(x->null.hmm, &n->null.R.super)) return error(DCP_EADDSTATE);
  if (imm_hmm_add_state(x->null.hmm, &n->null.G.super)) return error(DCP_EADDSTATE);
  if (imm_hmm_set_start(x->null.hmm, &n->null.F))       return error(DCP_ESETTRANS);
  if (imm_hmm_set_end(x->null.hmm, &n->null.G))         return error(DCP_ESETTRANS);

  if (imm_hmm_add_state(x->alt.hmm, &n->alt.S.super)) return error(DCP_EADDSTATE);
  if (imm_hmm_add_state(x->alt.hmm, &n->alt.N.super)) return error(DCP_EADDSTATE);
  if (imm_hmm_add_state(x->alt.hmm, &n->alt.B.super)) return error(DCP_EADDSTATE);
  if (imm_hmm_add_state(x->alt.hmm, &n->alt.E.super)) return error(DCP_EADDSTATE);
  if (imm_hmm_add_state(x->alt.hmm, &n->alt.J.super)) return error(DCP_EADDSTATE);
  if (imm_hmm_add_state(x->alt.hmm, &n->alt.C.super)) return error(DCP_EADDSTATE);
  if (imm_hmm_add_state(x->alt.hmm, &n->alt.T.super)) return error(DCP_EADDSTATE);
  if (imm_hmm_set_start(x->alt.hmm, &n->alt.S))       return error(DCP_ESETTRANS);
  if (imm_hmm_set_end(x->alt.hmm, &n->alt.T))         return error(DCP_ESETTRANS);

  return 0;
}

void init_xnodes(struct model *x)
{
  float e = x->params.epsilon;
  struct imm_nuclt_lprob const *nucltp = &x->null.nuclt_dist.nucltp;
  struct imm_codon_marg const *codonm = &x->null.nuclt_dist.codonm;
  struct model_xnode *n = &x->xnode;
  struct imm_nuclt const *nuclt = x->params.code->nuclt;

  imm_mute_state_init(&n->null.F, STATE_F, &nucltp->nuclt->super);
  struct imm_span w = imm_span(1, 5);
  imm_frame_state_init(&n->null.R, STATE_R, nucltp, codonm, e, w);
  // TODO: this is not xnode. Might refactor it?
  imm_frame_state_init(&x->null.state, STATE_R, &x->null.nuclt_dist.nucltp,
                       &x->null.nuclt_dist.codonm, e, w);
  imm_mute_state_init(&n->null.G, STATE_G, &nucltp->nuclt->super);

  imm_mute_state_init(&n->alt.S, STATE_S, &nuclt->super);
  imm_frame_state_init(&n->alt.N, STATE_N, nucltp, codonm, e, w);
  imm_mute_state_init(&n->alt.B, STATE_B, &nuclt->super);
  imm_mute_state_init(&n->alt.E, STATE_E, &nuclt->super);
  imm_frame_state_init(&n->alt.J, STATE_J, nucltp, codonm, e, w);
  imm_frame_state_init(&n->alt.C, STATE_C, nucltp, codonm, e, w);
  imm_mute_state_init(&n->alt.T, STATE_T, &nuclt->super);
}

void calculate_occupancy(struct model *x)
{
  struct trans *trans = x->alt.trans;
  x->alt.locc[0] = imm_lprob_add(trans->MI, trans->MM);
  for (int i = 1; i < x->core_size; ++i)
  {
    ++trans;
    float v0 = x->alt.locc[i - 1] + imm_lprob_add(trans->MM, trans->MI);
    float v1 = log1_p(x->alt.locc[i - 1]) + trans->DM;
    x->alt.locc[i] = imm_lprob_add(v0, v1);
  }

  float logZ = imm_lprob_zero();
  int n = x->core_size;
  for (int i = 0; i < x->core_size; ++i)
  {
    logZ = imm_lprob_add(logZ, x->alt.locc[i] + logf(n - i));
  }

  for (int i = 0; i < x->core_size; ++i)
  {
    x->alt.locc[i] -= logZ;
  }

  assert(!imm_lprob_is_nan(logZ));
}

bool have_called_setup(struct model *x) { return x->core_size > 0; }

bool have_finished_add(struct model const *x)
{
  int core_size = x->core_size;
  return x->alt.node_idx == core_size && x->alt.trans_idx == (core_size + 1);
}

void init_delete(struct imm_mute_state *state, struct model *x)
{
  int id = state_make_delete_id(x->alt.node_idx);
  imm_mute_state_init(state, id, &x->params.code->nuclt->super);
}

void init_insert(struct imm_frame_state *state, float epsilon,
                 struct nuclt_dist const *nucltd, int node_idx)
{
  float e = epsilon;
  int id = state_make_insert_id(node_idx);
  struct imm_nuclt_lprob const *nucltp = &nucltd->nucltp;
  struct imm_codon_marg const *codonm = &nucltd->codonm;

  imm_frame_state_init(state, id, nucltp, codonm, e, imm_span(1, 5));
}

void init_match(struct imm_frame_state *state, struct model *x,
                struct nuclt_dist *d)
{
  float e = x->params.epsilon;
  int id = state_make_match_id(x->alt.node_idx);
  imm_frame_state_init(state, id, &d->nucltp, &d->codonm, e, imm_span(1, 5));
}

int init_null_xtrans(struct imm_hmm *hmm, struct model_xnode_null *n)
{
  if (imm_hmm_set_trans(hmm, &n->F.super, &n->R.super, LOG1)) return error(DCP_ESETTRANS);
  if (imm_hmm_set_trans(hmm, &n->R.super, &n->R.super, LOG1)) return error(DCP_ESETTRANS);
  if (imm_hmm_set_trans(hmm, &n->R.super, &n->G.super, LOG1)) return error(DCP_ESETTRANS);
  return 0;
}

int init_alt_xtrans(struct imm_hmm *hmm, struct model_xnode_alt *n)
{
  if (imm_hmm_set_trans(hmm, &n->S.super, &n->B.super, LOG1)) return error(DCP_ESETTRANS);
  if (imm_hmm_set_trans(hmm, &n->S.super, &n->N.super, LOG1)) return error(DCP_ESETTRANS);
  if (imm_hmm_set_trans(hmm, &n->N.super, &n->N.super, LOG1)) return error(DCP_ESETTRANS);
  if (imm_hmm_set_trans(hmm, &n->N.super, &n->B.super, LOG1)) return error(DCP_ESETTRANS);

  if (imm_hmm_set_trans(hmm, &n->E.super, &n->T.super, LOG1)) return error(DCP_ESETTRANS);
  if (imm_hmm_set_trans(hmm, &n->E.super, &n->C.super, LOG1)) return error(DCP_ESETTRANS);
  if (imm_hmm_set_trans(hmm, &n->C.super, &n->C.super, LOG1)) return error(DCP_ESETTRANS);
  if (imm_hmm_set_trans(hmm, &n->C.super, &n->T.super, LOG1)) return error(DCP_ESETTRANS);

  if (imm_hmm_set_trans(hmm, &n->E.super, &n->B.super, LOG1)) return error(DCP_ESETTRANS);
  if (imm_hmm_set_trans(hmm, &n->E.super, &n->J.super, LOG1)) return error(DCP_ESETTRANS);
  if (imm_hmm_set_trans(hmm, &n->J.super, &n->J.super, LOG1)) return error(DCP_ESETTRANS);
  if (imm_hmm_set_trans(hmm, &n->J.super, &n->B.super, LOG1)) return error(DCP_ESETTRANS);

  return 0;
}

struct imm_nuclt_lprob nuclt_lprob(struct imm_gencode const *gc,
                                   struct imm_codon_lprob const *codonp)
{
  float lprobs[] = {[0 ... IMM_NUCLT_SIZE - 1] = IMM_LPROB_ZERO};

  float const norm = logf(3);
  for (int i = 0; i < imm_gencode_size(gc); ++i)
  {
    struct imm_codon codon = imm_gencode_codon(gc, i);
    /* Check for FIXME-1 for an explanation of this
     * temporary hacky */
    codon.nuclt = codonp->nuclt;
    float lprob = imm_codon_lprob_get(codonp, codon);
    lprobs[codon.a] = imm_lprob_add(lprobs[codon.a], lprob - norm);
    lprobs[codon.b] = imm_lprob_add(lprobs[codon.b], lprob - norm);
    lprobs[codon.c] = imm_lprob_add(lprobs[codon.c], lprob - norm);
  }
  return imm_nuclt_lprob(codonp->nuclt, lprobs);
}

struct imm_codon_lprob codon_lprob(struct imm_gencode const *gc,
                                   struct imm_amino const *amino,
                                   struct imm_amino_lprob const *aminop,
                                   struct imm_nuclt const *nuclt)
{
  /* FIXME: We don't need 255 positions*/
  int count[] = {[0 ... 254] = 0};

  for (int i = 0; i < imm_gencode_size(gc); ++i)
    count[(int)imm_gencode_amino(gc, i)] += 1;

  struct imm_abc const *abc = &amino->super;
  /* TODO: We don't need 255 positions*/
  float lprobs[] = {[0 ... 254] = IMM_LPROB_ZERO};
  for (int i = 0; i < imm_abc_size(abc); ++i)
  {
    char aa = imm_abc_symbols(abc)[i];
    float norm = logf((float)count[(int)aa]);
    lprobs[(int)aa] = imm_amino_lprob_get(aminop, aa) - norm;
  }

  /* FIXME-1: imm_gc module assumes imm_dna_iupac as alphabet, we have to make
   * it configurable. For now I will assume that the calle of this
   * function is using imm_nuclt base of an imm_dna_iupac compatible alphabet
   */
  /* struct imm_codon_lprob codonp = imm_codon_lprob(nuclt); */
  struct imm_codon_lprob codonp = imm_codon_lprob(&imm_gencode_dna->super);
  for (int i = 0; i < imm_gencode_size(gc); ++i)
  {
    char aa = imm_gencode_amino(gc, i);
    imm_codon_lprob_set(&codonp, imm_gencode_codon(gc, i), lprobs[(int)aa]);
  }
  codonp.nuclt = nuclt;
  return codonp;
}

void setup_nuclt_dist(struct imm_gencode const *gc, struct nuclt_dist *dist,
                      struct imm_amino const *amino,
                      struct imm_nuclt const *nuclt,
                      float const lprobs[IMM_AMINO_SIZE])
{
  dist->nucltp = imm_nuclt_lprob(nuclt, uniform_lprobs);
  struct imm_amino_lprob aminop = imm_amino_lprob(amino, lprobs);
  struct imm_codon_lprob codonp =
      codon_lprob(gc, amino, &aminop, dist->nucltp.nuclt);
  imm_codon_lprob_normalize(&codonp);

  dist->nucltp = nuclt_lprob(gc, &codonp);
  dist->codonm = imm_codon_marg(&codonp);
}

int setup_entry_trans(struct model *x)
{
  if (x->params.entry_dist == ENTRY_DIST_UNIFORM)
  {
    float M = (float)x->core_size;
    float cost = logf(2.0 / (M * (M + 1))) * M;

    struct imm_state *B = &x->xnode.alt.B.super;
    for (int i = 0; i < x->core_size; ++i)
    {
      struct model_node *node = x->alt.nodes + i;
      x->BMk[i] = cost;
      if (imm_hmm_set_trans(x->alt.hmm, B, &node->M.super, cost))
        return error(DCP_ESETTRANS);
    }
  }
  else
  {
    assert(x->params.entry_dist == ENTRY_DIST_OCCUPANCY);
    calculate_occupancy(x);
    struct imm_state *B = &x->xnode.alt.B.super;
    for (int i = 0; i < x->core_size; ++i)
    {
      struct model_node *node = x->alt.nodes + i;
      x->BMk[i] = x->alt.locc[i];
      if (imm_hmm_set_trans(x->alt.hmm, B, &node->M.super, x->alt.locc[i]))
        return error(DCP_ESETTRANS);
    }
  }
  return 0;
}

int setup_exit_trans(struct model *x)
{
  struct imm_state *E = &x->xnode.alt.E.super;

  for (int i = 0; i < x->core_size; ++i)
  {
    struct model_node *node = x->alt.nodes + i;
    if (imm_hmm_set_trans(x->alt.hmm, &node->M.super, E, logf(1)))
      return error(DCP_ESETTRANS);
  }
  for (int i = 1; i < x->core_size; ++i)
  {
    struct model_node *node = x->alt.nodes + i;
    if (imm_hmm_set_trans(x->alt.hmm, &node->D.super, E, logf(1)))
      return error(DCP_ESETTRANS);
  }
  return 0;
}

int setup_transitions(struct model *x)
{
  struct imm_hmm *h = x->alt.hmm;
  struct trans *trans = x->alt.trans;

  struct imm_state *B = &x->xnode.alt.B.super;
  struct imm_state *M1 = &x->alt.nodes[0].M.super;
  if (imm_hmm_set_trans(h, B, M1, trans[0].MM)) return error(DCP_ESETTRANS);

  for (int i = 0; i + 1 < x->core_size; ++i)
  {
    struct model_node *p = x->alt.nodes + i;
    struct model_node *n = x->alt.nodes + i + 1;
    int j = i + 1;
    struct trans t = trans[j];
    if (imm_hmm_set_trans(h, &p->M.super, &p->I.super, t.MI)) return error(DCP_ESETTRANS);
    if (imm_hmm_set_trans(h, &p->I.super, &p->I.super, t.II)) return error(DCP_ESETTRANS);
    if (imm_hmm_set_trans(h, &p->M.super, &n->M.super, t.MM)) return error(DCP_ESETTRANS);
    if (imm_hmm_set_trans(h, &p->I.super, &n->M.super, t.IM)) return error(DCP_ESETTRANS);
    if (imm_hmm_set_trans(h, &p->M.super, &n->D.super, t.MD)) return error(DCP_ESETTRANS);
    if (imm_hmm_set_trans(h, &p->D.super, &n->D.super, t.DD)) return error(DCP_ESETTRANS);
    if (imm_hmm_set_trans(h, &p->D.super, &n->M.super, t.DM)) return error(DCP_ESETTRANS);
  }

  int n = x->core_size;
  struct imm_state *Mm = &x->alt.nodes[n - 1].M.super;
  struct imm_state *E = &x->xnode.alt.E.super;
  if (imm_hmm_set_trans(h, Mm, E, trans[n].MM)) return error(DCP_ESETTRANS);

  if (setup_entry_trans(x))                          return error(DCP_ESETTRANS);
  if (setup_exit_trans(x))                           return error(DCP_ESETTRANS);
  if (init_null_xtrans(x->null.hmm, &x->xnode.null)) return error(DCP_ESETTRANS);
  return init_alt_xtrans(x->alt.hmm, &x->xnode.alt);
}
