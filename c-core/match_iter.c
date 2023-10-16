#include "match_iter.h"
#include "match.h"

void match_iter_init(struct match_iter *x, struct imm_seq const *seq,
                     struct imm_path const *path)
{
  x->seq = seq;
  x->path = path;
  x->idx = -1;
  x->offset = 0;
}

int match_iter_next(struct match_iter *x, struct match *match)
{
  x->idx += 1;
  if (match_iter_end(x)) return 0;

  struct imm_path const *path = x->path;

  struct imm_step const *step = imm_path_step(path, x->idx);
  struct imm_range range = imm_range(x->offset, x->offset + step->seqsize);
  struct imm_seq seq = imm_seq_slice(x->seq, range);

  x->offset += step->seqsize;

  return match_setup(match, *step, seq);
}

bool match_iter_end(struct match_iter const *x)
{
  return x->idx >= (int)imm_path_nsteps(x->path);
}
