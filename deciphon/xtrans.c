#include "xtrans.h"

void dcp_xtrans_init(struct dcp_xtrans *t)
{
  t->NN = IMM_LPROB_ONE;
  t->NB = IMM_LPROB_ONE;
  t->EC = IMM_LPROB_ONE;
  t->CC = IMM_LPROB_ONE;
  t->CT = IMM_LPROB_ONE;
  t->EJ = IMM_LPROB_ONE;
  t->JJ = IMM_LPROB_ONE;
  t->JB = IMM_LPROB_ONE;
  t->RR = IMM_LPROB_ONE;
}
