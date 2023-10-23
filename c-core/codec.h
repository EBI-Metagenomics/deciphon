#ifndef CODEC_H
#define CODEC_H

#include <stdbool.h>

struct imm_codon;
struct imm_path;
struct imm_seq;
struct protein;

struct codec
{
  int idx;
  int start;
  struct protein const *protein;
  struct imm_path const *path;
};

// clang-format off
struct codec codec_init(struct protein const *, struct imm_path const *);
int          codec_next(struct codec *, struct imm_seq const *,
                        struct imm_codon *);
bool         codec_end(struct codec const *);
// clang-format on

#endif
