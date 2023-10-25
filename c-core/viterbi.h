#ifndef VITERBI_H
#define VITERBI_H

#include <stdbool.h>

struct imm_eseq;
struct protein;
struct viterbi;

// clang-format off
void  viterbi_init(struct viterbi *);
int   viterbi_setup(struct viterbi *, struct protein const *, struct imm_eseq const *);
void  viterbi_cleanup(struct viterbi *);
float viterbi_null_loglik(struct viterbi *);
float viterbi_alt_loglik(struct viterbi *);
int   viterbi_alt_path(struct viterbi *, float *loglik);
// clang-format on

#endif
