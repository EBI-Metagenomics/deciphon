#ifndef CODEC_H
#define CODEC_H

#include <stdbool.h>

struct imm_codon;
struct imm_path;
struct imm_seq;
struct decoder;

struct codec
{
  int idx;
  int start;
  struct decoder const *decoder;
  struct imm_path const *path;
};

struct codec codec_init(struct decoder const *, struct imm_path const *);
int          codec_next(struct codec *, struct imm_seq const *, struct imm_codon *);
bool         codec_end(struct codec const *);

#endif
