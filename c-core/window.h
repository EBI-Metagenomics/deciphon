#ifndef WINDOW_H
#define WINDOW_H

#include "compiler.h"
#include "imm/imm.h"
#include "sequence.h"
#include <stdbool.h>

struct window
{
  int core_size;
  struct sequence const *seq;
  struct sequence iter;
  struct imm_range range;
  int id;
};

struct window window_setup(struct sequence const *, int core_size);
bool window_next(struct window *, int last_hit_pos);
struct sequence const *window_sequence(struct window const *);
int window_id(struct window const *);
struct imm_range window_range(struct window const *);

#endif
