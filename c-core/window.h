#ifndef DECIPHON_WINDOW_H
#define DECIPHON_WINDOW_H

#include "api.h"
#include "imm/imm.h"
#include "seq_struct.h"
#include <stdbool.h>

struct window
{
  int core_size;
  struct dcp_seq const *seq;
  struct dcp_seq iter;
  struct imm_range range;
  int id;
};

struct window window_setup(struct dcp_seq const *, int core_size);
bool window_next(struct window *, int last_hit_pos);
struct dcp_seq const *window_sequence(struct window const *);
int window_id(struct window const *);
struct imm_range window_range(struct window const *);

#endif
