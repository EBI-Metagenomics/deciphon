#ifndef DECIPHON_MODEL_H
#define DECIPHON_MODEL_H

#include "entry_dist.h"
#include "imm/imm.h"
#include "model_params.h"
#include "model_summary.h"
#include "node.h"
#include "nuclt_dist.h"
#include "rc.h"
#include "state.h"
#include "trans.h"
#include "xnode.h"
#include "xtrans.h"

enum
{
  DCP_MODEL_MAX = 4096,
};

struct dcp_model
{
  struct dcp_model_params params;
  unsigned core_size;
  struct dcp_xnode xnode;
  struct dcp_xtrans xtrans;
  char consensus[DCP_MODEL_MAX + 1];

  struct
  {
    float lprobs[IMM_AMINO_SIZE];
    struct dcp_nuclt_dist nuclt_dist;
    struct imm_frame_state state;
    struct imm_hmm hmm;
  } null;

  struct
  {
    struct dcp_nuclt_dist nuclt_dist;
    struct imm_frame_state state;
  } background;

  struct
  {
    unsigned node_idx;
    struct dcp_model_node *nodes;
    float *locc;
    unsigned trans_idx;
    struct dcp_trans *trans;
    struct imm_hmm hmm;

    struct
    {
      struct dcp_nuclt_dist nucltd;
    } insert;
  } alt;

  float *BMk;
};

int dcp_model_add_node(struct dcp_model *, float const lp[IMM_AMINO_SIZE],
                       char consensus);

int dcp_model_add_trans(struct dcp_model *, struct dcp_trans);

void dcp_model_cleanup(struct dcp_model const *);

void dcp_model_init(struct dcp_model *, struct dcp_model_params params,
                    float const null_lprobs[IMM_AMINO_SIZE]);

int dcp_model_setup(struct dcp_model *, unsigned core_size);

void dcp_model_write_dot(struct dcp_model const *, FILE *);

struct dcp_model_summary dcp_model_summary(struct dcp_model *);

#endif
