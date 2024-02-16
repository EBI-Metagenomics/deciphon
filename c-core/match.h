#ifndef MATCH_H
#define MATCH_H

#include "imm/codon.h"
#include "imm/step.h"
#include <stdbool.h>

struct protein;

struct match
{
  struct protein const *protein;
  struct imm_step step;
  struct imm_seq seq;
  struct imm_codon codon;
};

// clang-format off
struct match     match_init(struct protein const *);
int              match_setup(struct match *, struct imm_step, struct imm_seq);
int              match_state_name(struct match const *, char *dst);
bool             match_state_is_mute(struct match const *);
bool             match_state_is_core(struct match const *);
int              match_state_state_id(struct match const *);
char             match_amino(struct match const *);
struct imm_codon match_codon(struct match const *);
// clang-format on

#endif
