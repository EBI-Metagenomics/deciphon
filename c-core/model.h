#ifndef MODEL_H
#define MODEL_H

#include "entry_dist.h"
#include "imm/imm.h"
#include "model_node.h"
#include "model_params.h"
#include "model_xnode.h"
#include "nuclt_dist.h"
#include "rc.h"
#include "state.h"
#include "trans.h"
#include "xlimits.h"
#include "xtrans.h"

enum
{
  DCP_MODEL_MAX = DCP_CORE_SIZE,
};

struct model
{
  struct model_params params;
  int core_size;
  struct model_xnode xnode;
  struct xtrans xtrans;
  char consensus[DCP_MODEL_MAX + 1];

  struct
  {
    float lprobs[IMM_AMINO_SIZE];
    struct nuclt_dist nuclt_dist;
    struct imm_frame_state state;
    struct imm_hmm *hmm;
  } null;

  struct
  {
    struct nuclt_dist nuclt_dist;
    struct imm_frame_state state;
  } background;

  struct
  {
    int node_idx;
    struct model_node *nodes;
    float *locc;
    int trans_idx;
    struct trans *trans;
    struct imm_hmm *hmm;

    struct
    {
      struct nuclt_dist nucltd;
    } insert;
  } alt;

  float *BMk;
};

// clang-format off
int  model_add_node(struct model *, float const lprobs[], char consensus);
int  model_add_trans(struct model *, struct trans);
void model_cleanup(struct model *);
int  model_init(struct model *, struct model_params, float const null_lprobs[]);
int  model_setup(struct model *, int core_size);
void model_write_dot(struct model const *, FILE *);
// clang-format on

#endif
