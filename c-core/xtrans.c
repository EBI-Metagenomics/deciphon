#include "xtrans.h"
#include "array_size.h"
#include "imm_dump.h"
#include "imm_lprob.h"

void xtrans_init(struct xtrans *t)
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

void xtrans_dump(struct xtrans const *x, FILE *fp)
{
  float const arr[] = {x->NN, x->NB, x->EC, x->CC, x->CT,
                       x->EJ, x->JJ, x->JB, x->RR};
  imm_dump_array_f32(array_size(arr), arr, fp);
}
