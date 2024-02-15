#include "match_iter.h"
#include "imm/path.h"
#include "match.h"
#include <limits.h>

void match_iter_init(struct match_iter *x, struct imm_seq const *seq,
                     struct imm_path const *path)
{
  x->seq = seq;
  x->path = path;
  x->offset = 0;
  x->seqoffset = 0;
  x->end = false;
}

int match_iter_next(struct match_iter *x, struct match *match)
{
  if (x->offset == imm_path_nsteps(x->path))
  {
    x->end = true;
    return 0;
  }
  x->end = false;

  struct imm_path const *path = x->path;

  struct imm_step const *step = imm_path_step(path, x->offset);
  struct imm_range r = imm_range(x->seqoffset, x->seqoffset + step->seqsize);
  struct imm_seq seq = imm_seq_slice(x->seq, r);

  x->offset += 1;
  x->seqoffset += step->seqsize;

  return match_setup(match, *step, seq);
}

int match_iter_prev(struct match_iter *x, struct match *match)
{
  x->end = false;
  if (match_iter_begin(x)) return 0;

  struct imm_path const *path = x->path;

  x->offset -= 1;
  struct imm_step const *step = imm_path_step(path, x->offset);
  x->seqoffset -= step->seqsize;

  struct imm_range r = imm_range(x->seqoffset, x->seqoffset + step->seqsize);
  struct imm_seq seq = imm_seq_slice(x->seq, r);


  return match_setup(match, *step, seq);
}

bool match_iter_begin(struct match_iter const *x) { return x->offset == 0; }

bool match_iter_end(struct match_iter const *x) { return x->end; }

void match_iter_rewind(struct match_iter *x)
{
  x->offset = 0;
  x->seqoffset = 0;
  x->end = false;
}

int match_iter_tell(struct match_iter const *x) { return x->offset; }

int match_iter_seqtell(struct match_iter const *x) { return x->seqoffset; }

int match_iter_seek(struct match_iter *x, struct match *match, int offset)
{
  int rc = 0;
  match_iter_rewind(x);
  while (!(rc = match_iter_next(x, match)))
  {
    if (match_iter_end(x) || match_iter_tell(x) == offset) break;
  }
  return rc;
}
