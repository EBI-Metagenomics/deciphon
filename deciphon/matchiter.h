#ifndef DECIPHON_MATCHITER_H
#define DECIPHON_MATCHITER_H

#include <stdbool.h>

struct imm_path;
struct imm_seq;
struct dcp_match;

struct dcp_matchiter
{
  struct imm_seq const *seq;
  struct imm_path const *path;
  int idx;
  int offset;
};

void dcp_matchiter_init(struct dcp_matchiter *, struct imm_seq const *,
                        struct imm_path const *);
int dcp_matchiter_next(struct dcp_matchiter *, struct dcp_match *);
bool dcp_matchiter_end(struct dcp_matchiter const *);

#endif
