#ifndef SEQUENCE_H
#define SEQUENCE_H

#include "imm_eseq.h"
#include "imm_range.h"
#include "node.h"
#include <stdbool.h>

struct sequence
{
  long id;
  char const *name;
  char *data;
  struct
  {
    struct imm_seq seq;
    struct imm_eseq eseq;
  } imm;
  bool encoded;
  struct node node;
};

int             sequence_init(struct sequence *, long id, char const *name, char const *data);
int             sequence_encode(struct sequence *, struct imm_code const *);
void            sequence_cleanup(struct sequence *);
struct sequence sequence_slice(struct sequence const *, struct imm_range);
int             sequence_size(struct sequence const *);

#endif
