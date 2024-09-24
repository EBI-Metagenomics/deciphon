#ifndef BATCH_H
#define BATCH_H

struct imm_code;
struct dcp_batch;

int         batch_encode(struct dcp_batch const *, struct imm_code const *);
struct iter batch_iter(struct dcp_batch const *);

#endif
