#ifndef DECODER_H
#define DECODER_H

#include "nuclt_dist.h"

struct imm_seq;
struct imm_codon;
struct protein;

struct decoder
{
  float epsilon;
  struct nuclt_dist bg;
  struct nuclt_dist null;
  struct nuclt_dist *nodes;
  struct imm_gencode const *gencode;
  struct imm_nuclt_code const *code;
};

void decoder_init(struct decoder *);
int  decoder_setup(struct decoder *, struct protein *);
int  decoder_decode(struct decoder const *, struct imm_seq const *, int state_id,
                    struct imm_codon *);
void decoder_cleanup(struct decoder *);

#endif
