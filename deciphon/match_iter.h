#ifndef DECIPHON_MATCH_ITER_H
#define DECIPHON_MATCH_ITER_H

#include <stdbool.h>

struct imm_path;
struct imm_seq;
struct dcp_match;

struct dcp_match_iter
{
  struct imm_seq const *seq;
  struct imm_path const *path;
  int idx;
  int offset;
};

void dcp_match_iter_init(struct dcp_match_iter *, struct imm_seq const *,
                         struct imm_path const *);
int dcp_match_iter_next(struct dcp_match_iter *, struct dcp_match *);
bool dcp_match_iter_end(struct dcp_match_iter const *);

#endif
