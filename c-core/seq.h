#ifndef DECIPHON_SEQ_H
#define DECIPHON_SEQ_H

#include "api.h"
#include <stdbool.h>

struct dcp_seq;
struct imm_seq;
struct imm_eseq;
struct imm_code;

typedef bool dcp_seq_next_fn(struct dcp_seq *, void *);

DCP_API int dcp_seq_setup(struct dcp_seq *, long id, char const *name,
                          char const *data);
DCP_API void dcp_seq_cleanup(struct dcp_seq *);

void dcp_seq_init(struct dcp_seq *, struct imm_code const *);
struct imm_seq const *dcp_seq_immseq(struct dcp_seq const *);
struct imm_eseq const *dcp_seq_immeseq(struct dcp_seq const *);
long dcp_seq_id(struct dcp_seq const *);
unsigned dcp_seq_size(struct dcp_seq const *);
char const *dcp_seq_data(struct dcp_seq const *);

#endif
