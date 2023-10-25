#ifndef VITERBI_H
#define VITERBI_H

#include <stdbool.h>

struct imm_eseq;
struct protein;
struct viterbi;

// clang-format off
void viterbi_init(struct viterbi *);
int  viterbi_setup(struct viterbi *, struct protein const *, struct imm_eseq const *);
int  viterbi_setup_path(struct viterbi *);
void viterbi_cleanup(struct viterbi *);

float viterbi_null(struct protein *, struct imm_eseq const *);
float viterbi_alt(struct viterbi *, struct imm_eseq const *, bool const nopath);
int   viterbi_alt_extract_path(struct viterbi *, int seq_size);
// clang-format on

#endif
