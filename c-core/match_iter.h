#ifndef MATCH_ITER_H
#define MATCH_ITER_H

#include <stdbool.h>

struct imm_path;
struct imm_seq;
struct match;

struct match_iter
{
  struct imm_seq const *seq;
  struct imm_path const *path;
  int offset;
  int seqoffset;
  bool end;
};

void match_iter_init(struct match_iter *, struct imm_seq const *, struct imm_path const *);
int  match_iter_next(struct match_iter *, struct match *);
int  match_iter_prev(struct match_iter *, struct match *);
bool match_iter_begin(struct match_iter const *);
bool match_iter_end(struct match_iter const *);
void match_iter_rewind(struct match_iter *);
int  match_iter_tell(struct match_iter const *);
int  match_iter_seqtell(struct match_iter const *);
int  match_iter_seek(struct match_iter *, struct match *, int offset);

#endif
