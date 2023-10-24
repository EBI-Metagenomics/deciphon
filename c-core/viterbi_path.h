#ifndef VITERBI_PATH_H
#define VITERBI_PATH_H

struct trellis;
struct imm_path;

int unzip_path(struct trellis *x, int seq_size, struct imm_path *path);

#endif
