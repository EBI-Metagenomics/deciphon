#ifndef XTRANS_H
#define XTRANS_H

#include <stdio.h>

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
void xtrans_dump(struct xtrans const *, FILE *restrict);

#endif
