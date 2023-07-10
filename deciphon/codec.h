#ifndef DECIPHON_CODEC_H
#define DECIPHON_CODEC_H

#include "rc.h"
#include <stdbool.h>

struct imm_codon;
struct imm_path;
struct imm_seq;
struct protein;

struct dcp_codec
{
  unsigned idx;
  unsigned start;
  struct protein const *protein;
  struct imm_path const *path;
};

struct dcp_codec dcp_codec_init(struct protein const *,
                                struct imm_path const *);
int dcp_codec_next(struct dcp_codec *, struct imm_seq const *,
                   struct imm_codon *);
bool dcp_codec_end(struct dcp_codec const *);

#endif
