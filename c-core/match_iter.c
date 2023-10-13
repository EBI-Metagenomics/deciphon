#include "match_iter.h"
#include "match.h"

void dcp_match_iter_init(struct dcp_match_iter *x, struct imm_seq const *seq,
                         struct imm_path const *path)
{
  x->seq = seq;
  x->path = path;
  x->idx = -1;
  x->offset = 0;
}

int dcp_match_iter_next(struct dcp_match_iter *x, struct dcp_match *match)
{
  x->idx += 1;
  if (dcp_match_iter_end(x)) return 0;

  struct imm_path const *path = x->path;

  struct imm_step const *step = imm_path_step(path, x->idx);
  struct imm_range range = imm_range(x->offset, x->offset + step->seqsize);
  struct imm_seq seq = imm_seq_slice(x->seq, range);

  x->offset += step->seqsize;

  return dcp_match_setup(match, *step, seq);
}

bool dcp_match_iter_end(struct dcp_match_iter const *x)
{
  return x->idx >= (int)imm_path_nsteps(x->path);
}
