#include "protein.h"
#include "protein_node.h"
#include "viterbi.h"
#include <math.h>

struct viterbi_xtrans
{
  float const SB;
  float const SN;
  float const NN;
  float const NB;

  float const ET;
  float const EC;
  float const CC;
  float const CT;

  float const EB;
  float const EJ;
  float const JJ;
  float const JB;

  float const ME;
  float const DE;
};

struct viterbi_xtrans viterbi_xtrans_init(struct xtrans x)
{
  return (struct viterbi_xtrans){
      .SB = x.NB,
      .SN = x.NN,
      .NN = x.NN,
      .NB = x.NB,

      .ET = x.EC + x.CT,
      .EC = x.EC + x.CC,
      .CC = x.CC,
      .CT = x.CT,

      .EB = x.EJ + x.JB,
      .EJ = x.EJ + x.JJ,
      .JJ = x.JJ,
      .JB = x.JB,

      .ME = 0.0f,
      .DE = 0.0f,
  };
}

int setup_viterbi(struct viterbi *x, struct protein const *protein)
{
  int K = protein->core_size;
  int rc = viterbi_setup(x, K);
  if (rc) return rc;

  struct viterbi_xtrans const xt = viterbi_xtrans_init(protein->xtrans);
  viterbi_set_extr_trans(x, EXTR_TRANS_RR, -protein->null.RR);
  viterbi_set_extr_trans(x, EXTR_TRANS_SN, -xt.SN);
  viterbi_set_extr_trans(x, EXTR_TRANS_NN, -xt.NN);
  viterbi_set_extr_trans(x, EXTR_TRANS_SB, -xt.SB);
  viterbi_set_extr_trans(x, EXTR_TRANS_NB, -xt.NB);
  viterbi_set_extr_trans(x, EXTR_TRANS_EB, -xt.EB);
  viterbi_set_extr_trans(x, EXTR_TRANS_JB, -xt.JB);
  viterbi_set_extr_trans(x, EXTR_TRANS_EJ, -xt.EJ);
  viterbi_set_extr_trans(x, EXTR_TRANS_JJ, -xt.JJ);
  viterbi_set_extr_trans(x, EXTR_TRANS_EC, -xt.EC);
  viterbi_set_extr_trans(x, EXTR_TRANS_CC, -xt.CC);
  viterbi_set_extr_trans(x, EXTR_TRANS_ET, -xt.ET);
  viterbi_set_extr_trans(x, EXTR_TRANS_CT, -xt.CT);

  for (int k = 0; k < K; ++k)
  {
    viterbi_set_core_trans(x, CORE_TRANS_BM, -protein->BMk[k], k);
  }

  viterbi_set_core_trans(x, CORE_TRANS_MM, INFINITY, 0);
  viterbi_set_core_trans(x, CORE_TRANS_MD, INFINITY, 0);
  viterbi_set_core_trans(x, CORE_TRANS_IM, INFINITY, 0);
  viterbi_set_core_trans(x, CORE_TRANS_DM, INFINITY, 0);
  viterbi_set_core_trans(x, CORE_TRANS_DD, INFINITY, 0);
  for (int k = 0; k < K - 1; ++k)
  {
    viterbi_set_core_trans(x, CORE_TRANS_MM, -protein->nodes[k].trans.MM, k + 1);
    viterbi_set_core_trans(x, CORE_TRANS_MI, -protein->nodes[k].trans.MI, k + 0);
    viterbi_set_core_trans(x, CORE_TRANS_MD, -protein->nodes[k].trans.MD, k + 1);
    viterbi_set_core_trans(x, CORE_TRANS_IM, -protein->nodes[k].trans.IM, k + 1);
    viterbi_set_core_trans(x, CORE_TRANS_II, -protein->nodes[k].trans.II, k + 0);
    viterbi_set_core_trans(x, CORE_TRANS_DM, -protein->nodes[k].trans.DM, k + 1);
    viterbi_set_core_trans(x, CORE_TRANS_DD, -protein->nodes[k].trans.DD, k + 1);
  }
  viterbi_set_core_trans(x, CORE_TRANS_MI, INFINITY, K - 1);
  viterbi_set_core_trans(x, CORE_TRANS_II, INFINITY, K - 1);

  for (size_t i = 0; i < DCP_ABC_TABLE_SIZE; ++i)
  {
    viterbi_set_null(x, -protein->null.emission[i], i);
    viterbi_set_background(x, -protein->bg.emission[i], i);

    for (int k = 0; k < K; ++k)
      viterbi_set_match(x, -protein->nodes[k].emission[i], k, i);
  }
  return 0;
}
