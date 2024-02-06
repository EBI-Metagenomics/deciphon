#ifndef SEQUENCE_H
#define SEQUENCE_H

#include "imm/eseq.h"
#include "imm/range.h"
#include "node.h"
#include <stdbool.h>

struct sequence
{
  long id;
  char const *name;
  char const *data;
  struct
  {
    struct imm_seq seq;
    struct imm_eseq eseq;
  } imm;
  struct node node;
};

int             sequence_init(struct sequence *, struct imm_code const *, long id, char const *name, char const *data);
void            sequence_cleanup(struct sequence *);
struct sequence sequence_slice(struct sequence const *, struct imm_range);
int             sequence_size(struct sequence const *);

#endif
