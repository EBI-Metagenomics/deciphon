#include "match.h"
#include "imm/gencode.h"
#include "imm/nuclt_code.h"
#include "protein.h"
#include "state.h"

struct match match_init(struct protein const *protein)
{
  return (struct match){protein, {}, {}, {}};
}

int match_setup(struct match *x, struct imm_step step, struct imm_seq seq)
{
  x->step = step;
  x->seq = seq;

  if (!state_is_mute(step.state_id))
  {
    x->codon = imm_codon_any(x->protein->params.code->nuclt);
    return protein_decode(x->protein, &seq, step.state_id, &x->codon);
  }
  return 0;
}

int match_state_name(struct match const *x, char *dst)
{
  return state_name(x->step.state_id, dst);
}

bool match_state_is_mute(struct match const *x)
{
  return state_is_mute(x->step.state_id);
}

bool match_state_is_core(struct match const *x)
{
  return state_is_match(x->step.state_id) ||
         state_is_insert(x->step.state_id) || state_is_delete(x->step.state_id);
}

int match_state_state_id(struct match const *x) { return x->step.state_id; }

char match_amino(struct match const *x)
{
  return imm_gencode_decode(x->protein->params.gencode, x->codon);
}

struct imm_codon match_codon(struct match const *x) { return x->codon; }
