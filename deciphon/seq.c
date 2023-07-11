#include "seq.h"
#include "imm/imm.h"
#include <stddef.h>
#include <stdlib.h>

struct dcp_seq
{
  long id;
  char const *name;
  char const *data;
  struct imm_seq imm_seq;
};

struct dcp_seq *dcp_seq_new(void)
{
  struct dcp_seq *x = malloc(sizeof(*x));
  if (!x) return NULL;
  x->id = 0;
  x->name = NULL;
  x->data = NULL;
  return x;
}

void dcp_seq_del(struct dcp_seq const *x)
{
  if (x) free((void *)x);
}

void dcp_seq_setup(struct dcp_seq *x, long id, char const *name,
                   char const *data)
{
  x->id = id;
  x->name = name;
  x->data = data;
}

void dcp_seq_set_abc(struct dcp_seq *x, struct imm_abc const *abc)
{
  x->imm_seq = imm_seq(imm_str(x->data), abc);
}

unsigned dcp_seq_size(struct dcp_seq *x) { return imm_seq_size(&x->imm_seq); }

char const *dcp_seq_data(struct dcp_seq *x) { return imm_seq_str(&x->imm_seq); }
