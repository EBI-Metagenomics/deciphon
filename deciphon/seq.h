#ifndef DECIPHON_SEQ_H
#define DECIPHON_SEQ_H

#include "api.h"
#include <stdbool.h>

struct imm_abc;
struct dcp_seq;

typedef bool dcp_seq_next_fn(struct dcp_seq *, void *);

DCP_API struct dcp_seq *dcp_seq_new(void);
DCP_API void dcp_seq_del(struct dcp_seq const *);
DCP_API void dcp_seq_setup(struct dcp_seq *, long id, char const *name,
                           char const *data);

void dcp_seq_set_abc(struct dcp_seq *, struct imm_abc const *);
unsigned dcp_seq_size(struct dcp_seq *);
char const *dcp_seq_data(struct dcp_seq *);

#endif
