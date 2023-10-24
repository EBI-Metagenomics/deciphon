#ifndef VITERBI_H
#define VITERBI_H

#include <stdbool.h>

struct imm_eseq;
struct protein;
struct viterbi_task;

// clang-format off
float viterbi_null(struct protein *, struct imm_eseq const *);
float viterbi_alt(struct viterbi_task *, struct imm_eseq const *, bool const nopath);
int   viterbi_alt_extract_path(struct viterbi_task *, int seq_size);
// clang-format on

#endif
