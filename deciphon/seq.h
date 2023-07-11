#ifndef DECIPHON_SEQ_H
#define DECIPHON_SEQ_H

#include "api.h"
#include <stdbool.h>

struct imm_abc;
struct dcp_seq;
struct imm_seq;
struct imm_eseq;

typedef bool dcp_seq_next_fn(struct dcp_seq *, void *);

DCP_API void dcp_seq_setup(struct dcp_seq *, long id, char const *name,
                           char const *data);

void dcp_seq_init(struct dcp_seq *);
int dcp_seq_set_abc(struct dcp_seq *, struct imm_abc const *);
struct imm_seq const *dcp_seq_imm_seq(struct dcp_seq const *);
struct imm_eseq const *dcp_seq_imm_eseq(struct dcp_seq const *);
long dcp_seq_id(struct dcp_seq const *);
unsigned dcp_seq_size(struct dcp_seq const *);
char const *dcp_seq_data(struct dcp_seq const *);

#endif
