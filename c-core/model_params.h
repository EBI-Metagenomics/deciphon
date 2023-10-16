#ifndef MODEL_PARAMS_H
#define MODEL_PARAMS_H

#include "entry_dist.h"

struct model_params
{
  struct imm_gencode const *gencode;
  struct imm_amino const *amino;
  struct imm_nuclt_code const *code;
  enum entry_dist entry_dist;
  float epsilon;
};

#endif
