#include "seq.h"
#include "imm/imm.h"
#include "rc.h"
#include "seq_struct.h"
#include <stddef.h>
#include <stdlib.h>

void dcp_seq_setup(struct dcp_seq *x, long id, char const *name,
                   char const *data)
{
  x->id = id;
  x->name = name;
  x->data = data;
}

void dcp_seq_init(struct dcp_seq *x)
{
  x->id = 0;
  x->name = NULL;
  x->data = NULL;
}

int dcp_seq_set_abc(struct dcp_seq *x, struct imm_abc const *abc)
{
  x->imm_seq = imm_seq(imm_str(x->data), abc);
  return imm_eseq_setup(&x->imm_eseq, &x->imm_seq) ? DCP_ESEQABC : 0;
}

struct imm_seq const *dcp_seq_imm_seq(struct dcp_seq const *x)
{
  return &x->imm_seq;
}

long dcp_seq_id(struct dcp_seq const *x) { return x->id; }

unsigned dcp_seq_size(struct dcp_seq const *x)
{
  return imm_seq_size(&x->imm_seq);
}

char const *dcp_seq_data(struct dcp_seq const *x)
{
  return imm_seq_str(&x->imm_seq);
}
