#ifndef MATCH_H
#define MATCH_H

#include <stdbool.h>
#include "imm_step.h"

struct imm_codon;
struct imm_path;
struct imm_seq;
struct decoder;

struct match
{
  struct imm_path const *path;
  struct imm_seq const *sequence;
  struct decoder const *decoder;
  int step;
  int sequence_position;
};

struct match match_begin(struct imm_path const *, struct imm_seq const *, struct decoder const *);
struct match match_end(void);
bool         match_equal(struct match, struct match);
struct match match_next(struct match const *);
int          match_state_name(struct match const *, char *dst);
bool         match_state_is_mute(struct match const *);
bool         match_state_is_core(struct match const *);
int          match_state_id(struct match const *);
int          match_amino(struct match const *, char *amino);
int          match_codon(struct match const *, struct imm_codon *);

struct imm_seq         match_subsequence(struct match const *);
struct imm_step const *match_step(struct match const *);

#endif
