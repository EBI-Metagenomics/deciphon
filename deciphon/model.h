#ifndef DECIPHON_MODEL_H
#define DECIPHON_MODEL_H

#include "entry_dist.h"
#include "imm/imm.h"
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
  MODEL_MAX = 4096,
};

struct dcp_model
{
  struct imm_gencode const *gencode;
  struct imm_amino const *amino;
  struct imm_nuclt_code const *code;
  enum dcp_entry_dist entry_dist;
  float epsilon;
  unsigned core_size;
  struct dcp_xnode xnode;
  struct dcp_xtrans xtrans;
  char consensus[MODEL_MAX + 1];

  struct
  {
    float lprobs[IMM_AMINO_SIZE];
    struct dcp_nuclt_dist nuclt_dist;
    struct imm_hmm hmm;
  } null;

  struct
  {
    unsigned node_idx;
    struct node *nodes;
    float *locc;
    unsigned trans_idx;
    struct dcp_trans *trans;
    struct imm_hmm hmm;

    struct
    {
      struct dcp_nuclt_dist nucltd;
    } insert;
  } alt;
};

int dcp_model_add_node(struct dcp_model *, float const lp[IMM_AMINO_SIZE],
                       char consensus);

int dcp_model_add_trans(struct dcp_model *, struct dcp_trans);

void dcp_model_del(struct dcp_model const *);

void dcp_model_init(struct dcp_model *, struct imm_gencode const *,
                    struct imm_amino const *, struct imm_nuclt_code const *,
                    enum dcp_entry_dist, float epsilon,
                    float const null_lprobs[IMM_AMINO_SIZE]);

int dcp_model_setup(struct dcp_model *, unsigned core_size);

void dcp_model_write_dot(struct dcp_model const *, FILE *);

struct imm_amino const *dcp_model_amino(struct dcp_model const *);
struct imm_nuclt const *dcp_model_nuclt(struct dcp_model const *);
struct dcp_model_summary dcp_model_summary(struct dcp_model *);

#endif
