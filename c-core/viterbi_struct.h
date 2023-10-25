#ifndef VITERBI_STRUCT_H
#define VITERBI_STRUCT_H

#include "imm/imm.h"
#include "trellis.h"
#include "viterbi_coredp.h"
#include "viterbi_dp.h"

struct protein;

struct viterbi
{
  struct protein const *protein;
  struct imm_eseq const *seq;
  DECLARE_DP(R);
  DECLARE_COREDP(dp);
  DECLARE_DP(S);
  DECLARE_DP(N);
  DECLARE_DP(B);
  DECLARE_DP(J);
  DECLARE_DP(E);
  DECLARE_DP(C);
  DECLARE_DP(T);
  struct trellis trellis;
};

#endif
