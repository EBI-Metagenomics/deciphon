#include "match.h"
#include "decoder.h"
#include "error.h"
#include "imm_gencode.h"
#include "imm_nuclt_code.h"
#include "imm_path.h"
#include "state.h"

struct match match_begin(struct imm_path const *path,
                         struct imm_seq const *sequence,
                         struct decoder const *decoder)
{
  return (struct match){
      .path = path, .sequence = sequence, .decoder = decoder, 0, 0};
}

struct match match_end(void)
{
  return (struct match){
      .path = NULL, .sequence = NULL, .decoder = NULL, -1, -1};
}

bool match_equal(struct match a, struct match b)
{
  return a.path == b.path && a.sequence == b.sequence &&
         a.decoder == b.decoder && a.step == b.step &&
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
                        .decoder = x->decoder,
                        step,
                        pos};
}

int match_state_name(struct match const *x, char *dst)
{
  return error(state_name(match_state_id(x), dst));
}

bool match_state_is_mute(struct match const *x)
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
  struct imm_codon codon = imm_codon_any(x->decoder->code->nuclt);
  int state_id = match_state_id(x);

  struct imm_step const *step = imm_path_step(x->path, x->step);
  int pos = x->sequence_position;
  struct imm_range range = imm_range(pos, pos + step->seqsize);
  struct imm_seq seq = imm_seq_slice(x->sequence, range);

  int rc = decoder_decode(x->decoder, &seq, state_id, &codon);
  if (rc) return error(rc);

  *amino = imm_gencode_decode(x->decoder->gencode, codon);
  return 0;
}

int match_codon(struct match const *x, struct imm_codon *codon)
{
  *codon = imm_codon_any(x->decoder->code->nuclt);
  int state_id = match_state_id(x);
  struct imm_seq seq = match_subsequence(x);
  return error(decoder_decode(x->decoder, &seq, state_id, codon));
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
