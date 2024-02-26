#include "match.h"
#include "imm/gencode.h"
#include "imm/nuclt_code.h"
#include "imm/path.h"
#include "protein.h"
#include "state.h"

struct match match_begin(struct imm_path const *path,
                       struct imm_seq const *sequence,
                       struct protein const *protein)
{
  return (struct match){
      .path = path, .sequence = sequence, .protein = protein, 0, 0};
}

struct match match_end(void)
{
  return (struct match){.path = NULL, .sequence = NULL, .protein = NULL, -1, -1};
}

bool match_equal(struct match a, struct match b)
{
  return a.path == b.path && a.sequence == b.sequence &&
         a.protein == b.protein && a.step == b.step &&
         a.sequence_position == b.sequence_position;
}

struct match match_next(struct match const *x)
{
  if (match_equal(*x, match_end())) return match_end();
  if (x->step + 1 == imm_path_nsteps(x->path)) return match_end();

  int pos = x->sequence_position + imm_path_step(x->path, x->step)->seqsize;
  int step = x->step + 1;
  return (struct match){.path = x->path,
                       .sequence = x->sequence,
                       .protein = x->protein,
                       step,
                       pos};
}

int match_state_name(struct match const *x, char *dst)
{
  return state_name(match_state_id(x), dst);
}

bool match_state_is_mutet(struct match const *x)
{
  return state_is_mute(match_state_id(x));
}

bool match_state_is_core(struct match const *x)
{
  int state_id = match_state_id(x);
  return state_is_match(state_id) || state_is_insert(state_id) ||
         state_is_delete(state_id);
}

int match_state_id(struct match const *x)
{
  return imm_path_step(x->path, x->step)->state_id;
}

int match_amino(struct match const *x, char *amino)
{
  struct imm_codon codon = imm_codon_any(x->protein->params.code->nuclt);
  int state_id = match_state_id(x);

  struct imm_step const *step = imm_path_step(x->path, x->step);
  int pos = x->sequence_position;
  struct imm_range range = imm_range(pos, pos + step->seqsize);
  struct imm_seq seq = imm_seq_slice(x->sequence, range);

  int rc = protein_decode(x->protein, &seq, state_id, &codon);
  if (rc) return rc;

  *amino = imm_gencode_decode(x->protein->params.gencode, codon);
  return 0;
}

int match_codon(struct match const *x, struct imm_codon *codon)
{
  *codon = imm_codon_any(x->protein->params.code->nuclt);
  int state_id = match_state_id(x);
  struct imm_seq seq = match_subsequence(x);
  return protein_decode(x->protein, &seq, state_id, codon);
}

struct imm_seq match_subsequence(struct match const *x)
{
  struct imm_step const *step = imm_path_step(x->path, x->step);
  int pos = x->sequence_position;
  struct imm_range range = imm_range(pos, pos + step->seqsize);
  return imm_seq_slice(x->sequence, range);
}

struct imm_step const *match_step(struct match const *x)
{
  return imm_path_step(x->path, x->step);
}
