#ifndef VITERBI_H
#define VITERBI_H

#include "viterbi_task.h"
#include <stdbool.h>
#include <stdio.h>

struct imm_eseq;
struct protein;

// clang-format off
float viterbi_null(struct protein *, struct imm_eseq const *);
int   viterbi(struct protein *, struct imm_eseq const *, struct viterbi_task *,
              bool const nopath);
void  viterbi_dump(struct protein *, FILE *);
void  viterbi_dump_dot(struct protein *, FILE *);
// clang-format on

#endif
