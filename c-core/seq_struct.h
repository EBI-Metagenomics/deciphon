#ifndef SEQ_STRUCT_H
#define SEQ_STRUCT_H

#include "imm/imm.h"
#include "node.h"

struct dcp_seq
{
  long id;
  char const *name;
  char const *data;
  struct imm_seq imm_seq;
  struct imm_eseq imm_eseq;
  struct node node;
};

#endif
