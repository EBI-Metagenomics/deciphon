#ifndef DECIPHON_MATCH_H
#define DECIPHON_MATCH_H

#include "imm/imm.h"
#include <stdbool.h>

struct p7;

struct dcp_match
{
  struct p7 const *protein;
  struct imm_step step;
  struct imm_seq seq;
  struct imm_codon codon;
};

void dcp_match_init(struct dcp_match *, struct p7 const *);
int dcp_match_setup(struct dcp_match *, struct imm_step, struct imm_seq);
void dcp_match_state_name(struct dcp_match const *, char *dst);
bool dcp_match_state_is_mute(struct dcp_match const *);
char dcp_match_amino(struct dcp_match const *);
struct imm_codon dcp_match_codon(struct dcp_match const *);

#endif
