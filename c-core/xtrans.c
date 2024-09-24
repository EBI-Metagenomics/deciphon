#include "xtrans.h"
#include "array_size.h"
#include "bug.h"
#include "imm_dump.h"
#include "imm_lprob.h"
#include "viterbi.h"

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

void xtrans_setup(struct xtrans *x, bool multi_hits, bool hmmer3_compat,
                  int seq_size)
{
  BUG_ON(seq_size <= 0);

  float L = (float)seq_size;

  float q = 0.0;
  float log_q = IMM_LPROB_ZERO;

  if (multi_hits)
  {
    q = 0.5;
    log_q = log(0.5);
  }

  float lp  = log(L) - log(L + 2 + q / (1 - q));
  float l1p = log(2 + q / (1 - q)) - log(L + 2 + q / (1 - q));
  float lr  = log(L) - log(L + 1);

  x->NN = x->CC = x->JJ = lp;
  x->NB = x->CT = x->JB = l1p;
  x->RR = lr;
  x->EJ = log_q;
  x->EC = log(1 - q);

  if (hmmer3_compat)
  {
    x->NN = x->CC = x->JJ = logf(1);
  }
}

void xtrans_setup_viterbi(struct xtrans const *x, struct viterbi *v)
{
  viterbi_set_extr_trans(v, EXTR_TRANS_RR, -x->RR);
  viterbi_set_extr_trans(v, EXTR_TRANS_SN, -0 - x->NN);
  viterbi_set_extr_trans(v, EXTR_TRANS_NN, -x->NN);
  viterbi_set_extr_trans(v, EXTR_TRANS_SB, -0 - x->NB);
  viterbi_set_extr_trans(v, EXTR_TRANS_NB, -x->NB);
  viterbi_set_extr_trans(v, EXTR_TRANS_EB, -x->EJ - x->JB);
  viterbi_set_extr_trans(v, EXTR_TRANS_JB, -x->JB);
  viterbi_set_extr_trans(v, EXTR_TRANS_EJ, -x->EJ - x->JJ);
  viterbi_set_extr_trans(v, EXTR_TRANS_JJ, -x->JJ);
  viterbi_set_extr_trans(v, EXTR_TRANS_EC, -x->EC - x->CC);
  viterbi_set_extr_trans(v, EXTR_TRANS_CC, -x->CC);
  viterbi_set_extr_trans(v, EXTR_TRANS_ET, -x->EC - x->CT);
  viterbi_set_extr_trans(v, EXTR_TRANS_CT, -x->CT);
}

void xtrans_dump(struct xtrans const *x, FILE *fp)
{
  float const arr[] = {x->NN, x->NB, x->EC, x->CC, x->CT,
                       x->EJ, x->JJ, x->JB, x->RR};
  imm_dump_array_f32(array_size(arr), arr, fp);
}
