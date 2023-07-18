#ifndef DECIPHON_MODEL_PARAMS_H
#define DECIPHON_MODEL_PARAMS_H

#include "entry_dist.h"

struct dcp_model_params
{
  struct imm_gencode const *gencode;
  struct imm_amino const *amino;
  struct imm_nuclt_code const *code;
  enum dcp_entry_dist entry_dist;
  float epsilon;
};

static inline struct dcp_model_params
dcp_model_params(struct imm_gencode const *gencode,
                 struct imm_amino const *amino,
                 struct imm_nuclt_code const *code,
                 enum dcp_entry_dist entry_dist, float epsilon)
{
  return (struct dcp_model_params){gencode, amino, code, entry_dist, epsilon};
}

#endif
