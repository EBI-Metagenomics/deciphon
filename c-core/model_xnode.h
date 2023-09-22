#ifndef DECIPHON_MODEL_XNODE_H
#define DECIPHON_MODEL_XNODE_H

#include "imm/imm.h"

struct dcp_model_xnode_null
{
  struct imm_mute_state F;
  struct imm_frame_state R;
  struct imm_mute_state G;
};

struct dcp_model_xnode_alt
{
  struct imm_mute_state S;
  struct imm_frame_state N;
  struct imm_mute_state B;
  struct imm_mute_state E;
  struct imm_frame_state J;
  struct imm_frame_state C;
  struct imm_mute_state T;
};

struct dcp_model_xnode
{
  struct dcp_model_xnode_null null;
  struct dcp_model_xnode_alt alt;
};

#endif