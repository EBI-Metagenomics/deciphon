#ifndef DECIPHON_XTRANS_H
#define DECIPHON_XTRANS_H

#include <stdio.h>

struct dcp_xtrans
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

void dcp_xtrans_init(struct dcp_xtrans *);
void dcp_xtrans_dump(struct dcp_xtrans const *, FILE *restrict);

#endif
