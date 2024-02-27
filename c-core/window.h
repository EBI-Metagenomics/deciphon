#ifndef WINDOW_H
#define WINDOW_H

#include "sequence.h"
#include <stdbool.h>

struct window
{
  int core_size;
  struct sequence const *seq;
  struct sequence iter;
  struct imm_range range;
  int idx;
  int last_hit_pos;
};

struct window          window_setup(struct sequence const *, int core_size);
bool                   window_next(struct window *);
struct sequence const *window_sequence(struct window const *);
int                    window_idx(struct window const *);
struct imm_range       window_range(struct window const *);
void                   window_set_last_hit_position(struct window *, int pos);

#endif
