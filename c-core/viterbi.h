#ifndef VITERBI_H
#define VITERBI_H

#include <stdbool.h>

struct imm_eseq;
struct protein;
struct viterbi_task;

// clang-format off
float viterbi_null(struct protein *, struct imm_eseq const *);
int   viterbi(struct protein *, struct imm_eseq const *, struct viterbi_task *, bool const nopath);
// clang-format on

#endif
