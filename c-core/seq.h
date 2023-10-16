#ifndef SEQ_H
#define SEQ_H

#include "compiler.h"
#include "imm/imm.h"
#include <stdbool.h>

struct seq;
struct imm_seq;
struct imm_eseq;
struct imm_code;

typedef bool seq_next_fn(struct seq *, void *);

DCP_API int seq_setup(struct seq *, long id, char const *name,
                      char const *data);
DCP_API void seq_cleanup(struct seq *);

void seq_init(struct seq *, struct imm_code const *);
struct imm_seq const *seq_immseq(struct seq const *);
struct imm_eseq const *seq_immeseq(struct seq const *);
long seq_id(struct seq const *);
int seq_size(struct seq const *);
char const *seq_data(struct seq const *);
struct seq *seq_clone(struct seq *);
struct seq seq_slice(struct seq const *, struct imm_range);

#endif
