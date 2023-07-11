#include "matchiter.h"
#include "match.h"
#include "protein.h"

void dcp_matchiter_init(struct dcp_matchiter *x, struct imm_seq const *seq,
                        struct imm_path const *path)
{
  x->seq = seq;
  x->path = path;
  x->idx = -1;
  x->offset = 0;
}

int dcp_matchiter_next(struct dcp_matchiter *x, struct dcp_match *match)
{
  x->idx += 1;
  if (dcp_matchiter_end(x)) return 0;

  struct imm_path const *path = x->path;

  struct imm_step const *step = imm_path_step(path, x->idx);
  struct imm_seq seq = imm_subseq(x->seq, x->offset, step->seqlen);

  x->offset += step->seqlen;

  return dcp_match_setup(match, *step, seq);
}

bool dcp_matchiter_end(struct dcp_matchiter const *x)
{
  return x->idx >= (int)imm_path_nsteps(x->path);
}
