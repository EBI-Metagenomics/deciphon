#include "match.h"
#include "protein.h"
#include "state.h"
#include <string.h>

void dcp_match_init(struct dcp_match *x, struct dcp_protein const *protein)
{
  x->protein = protein;
}

int dcp_match_setup(struct dcp_match *x, struct imm_step step,
                    struct imm_seq seq)
{
  x->step = step;
  x->seq = seq;

  if (!dcp_state_is_mute(step.state_id))
  {
    x->codon = imm_codon_any(x->protein->params.code->nuclt);
    return dcp_protein_decode(x->protein, &seq, step.state_id, &x->codon);
  }
  return 0;
}

void dcp_match_state_name(struct dcp_match const *x, char *dst)
{
  x->protein->state_name(x->step.state_id, dst);
}

bool dcp_match_state_is_mute(struct dcp_match const *x)
{
  return dcp_state_is_mute(x->step.state_id);
}

char dcp_match_amino(struct dcp_match const *x)
{
  return imm_gencode_decode(x->protein->params.gencode, x->codon);
}

struct imm_codon dcp_match_codon(struct dcp_match const *x) { return x->codon; }
