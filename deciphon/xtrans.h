#ifndef DECIPHON_XTRANS_H
#define DECIPHON_XTRANS_H

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

#endif
