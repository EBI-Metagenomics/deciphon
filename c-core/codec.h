#ifndef DECIPHON_CODEC_H
#define DECIPHON_CODEC_H

#include <stdbool.h>

struct imm_codon;
struct imm_path;
struct imm_seq;
struct dcp_protein;

struct dcp_codec
{
  int idx;
  int start;
  struct dcp_protein const *protein;
  struct imm_path const *path;
};

struct dcp_codec dcp_codec_init(struct dcp_protein const *,
                                struct imm_path const *);
int dcp_codec_next(struct dcp_codec *, struct imm_seq const *,
                   struct imm_codon *);
bool dcp_codec_end(struct dcp_codec const *);

#endif
