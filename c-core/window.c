#include "window.h"
#include "bug.h"
#include "max.h"
#include "min.h"
#include "sequence.h"

struct window window_setup(struct sequence const *x, int core_size)
{
  struct imm_range r = imm_range(-1, 0);
  return (struct window){core_size, x, {}, r, -1, -1};
}

bool window_next(struct window *x)
{
  if (x->range.stop == sequence_size(x->seq)) return false;

  BUG_ON(!(-1 <= x->last_hit_pos && x->last_hit_pos < imm_range_size(x->range)));

  // We want to avoid miss hits. So lets compute an interval
  // that would include such a missed hit.
  int stop_miss = x->range.stop + 1;
  int start_miss = max(x->range.start + 1, x->range.start + x->last_hit_pos + 1);
  // Lets shorten that interval by assuming that the largest hit
  // would have size x->core_size * 4.
  start_miss = max(start_miss, stop_miss - x->core_size * 4);
  // We cannot go over the sequence size.
  BUG_ON(stop_miss > sequence_size(x->seq));

  x->range.start = start_miss;
  x->range.stop = min(start_miss + x->core_size * 50, sequence_size(x->seq));

  x->iter = sequence_slice(x->seq, x->range);
  x->idx += 1;
  BUG_ON(sequence_size(&x->iter) <= 0);
  return true;
}

struct sequence const *window_sequence(struct window const *x)
{
  return sequence_size(&x->iter) ? &x->iter : NULL;
}

int window_idx(struct window const *x) { return x->idx; }

struct imm_range window_range(struct window const *x) { return x->range; }

void window_set_last_hit_position(struct window *x, int pos)
{
  x->last_hit_pos = pos;
}
