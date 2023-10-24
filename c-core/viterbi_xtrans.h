#ifndef VITERBI_XTRANS_H
#define VITERBI_XTRANS_H

#include "compiler.h"
#include "xtrans.h"

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

CONST struct viterbi_xtrans viterbi_xtrans_init(struct xtrans x)
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

#endif
