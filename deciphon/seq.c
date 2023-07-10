#include "seq.h"

void dcp_seq_init(struct dcp_seq *seq, long id, char const *name,
                  char const *data)
{
  seq->id = id;
  seq->name = name;
  seq->data = data;
}
