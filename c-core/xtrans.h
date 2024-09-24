#ifndef XTRANS_H
#define XTRANS_H

#include <stdbool.h>
#include <stdio.h>

struct viterbi;

struct xtrans
{
  float NN;
  float CC;
  float JJ;
  float NB;
  float CT;
  float JB;
  float RR;
  float EJ;
  float EC;
};

void xtrans_init(struct xtrans *);
void xtrans_setup(struct xtrans *, bool multi_hits, bool hmmer3_compat, int seq_size);
void xtrans_setup_viterbi(struct xtrans const *, struct viterbi *);
void xtrans_dump(struct xtrans const *, FILE *);

#endif
