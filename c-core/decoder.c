#include "decoder.h"
#include "error.h"
#include "imm_frame_cond.h"
#include "imm_frame_epsilon.h"
#include "imm_lprob.h"
#include "protein.h"
#include "protein_node.h"
#include "state.h"
#include "xrealloc.h"
#include <stdlib.h>

void decoder_init(struct decoder *x)
{
  x->epsilon = -1;
  x->nodes   = NULL;
  x->gencode = NULL;
  x->code    = NULL;
}

int decoder_setup(struct decoder *x, struct protein *protein)
{
  x->epsilon = protein->params.epsilon;
  x->bg      = protein->bg.nuclt_dist;
  x->null    = protein->null.nuclt_dist;
  x->gencode = protein->params.gencode;
  x->code    = protein->params.code;

  x->nodes = xrealloc(x->nodes, (protein->core_size + 1) * sizeof(*x->nodes));
  if (!x->nodes) return error(DCP_ENOMEM);

  for (int i = 0; i <= protein->core_size; ++i)
    x->nodes[i] = protein->nodes[i].nuclt_dist;

  return 0;
}

int decoder_decode(struct decoder const *x, struct imm_seq const *seq,
                   int state_id, struct imm_codon *codon)
{
  if (state_is_mute(state_id)) return error(DCP_EINVALSTATE);

  struct nuclt_dist const *nucltd = NULL;
  if (state_is_insert(state_id))
    nucltd = &x->bg;
  else if (state_is_match(state_id))
    nucltd = &x->nodes[state_core_idx(state_id)];
  else
    nucltd = &x->null;

  struct imm_frame_cond cond = {imm_frame_epsilon(x->epsilon), &nucltd->nucltp,
                                &nucltd->codonm};

  if (imm_lprob_is_nan(imm_frame_cond_decode(&cond, seq, codon)))
    return error(DCP_EDECODON);

  return 0;
}

void decoder_cleanup(struct decoder *x)
{
  free(x->nodes);
  x->nodes = NULL;
}
