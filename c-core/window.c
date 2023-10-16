#include "window.h"
#include "seq.h"

// We are respecting HMMER's limit on sequence size:
//   https://github.com/EddyRivasLab/hmmer/blob/
//   9acd8b6758a0ca5d21db6d167e0277484341929b/src/p7_pipeline.c#L714
static int const WINDOW_SIZE = 100000 * 3;

IMM_CONST int max_size(int core_size) { return core_size * 3 * 3; }

struct window window_setup(struct seq const *x, int core_size)
{
  struct imm_range r = imm_range(0, 0);
  return (struct window){core_size, x, {}, r, -1};
}

bool window_next(struct window *x, int last_hit_pos)
{
  if (x->range.stop == seq_size(x->seq)) return false;

  assert(-1 <= last_hit_pos && last_hit_pos < imm_range_size(x->range));

  // We want to avoid miss hits. So lets compute an interval
  // that would include such a missed hit.
  int stop_miss = x->range.stop + 1;
  int start_miss = x->range.start + last_hit_pos + 1;
  // Lets shorten that interval by assuming that the largest hit
  // would have size max_size(x->core_size).
  start_miss =
      imm_max(start_miss, imm_max(stop_miss - max_size(x->core_size), 0));
  // We cannot go over the sequence size.
  assert(stop_miss <= dcp_seq_size(x->seq));

  x->range.start = start_miss;
  x->range.stop = imm_min(start_miss + WINDOW_SIZE, seq_size(x->seq));

  x->iter = seq_slice(x->seq, x->range);
  x->id += 1;
  assert(dcp_seq_size(&x->iter) > 0);
  return true;
}

struct seq const *window_sequence(struct window const *x)
{
  return seq_size(&x->iter) ? &x->iter : NULL;
}

int window_id(struct window const *x) { return x->id; }

struct imm_range window_range(struct window const *x) { return x->range; }
