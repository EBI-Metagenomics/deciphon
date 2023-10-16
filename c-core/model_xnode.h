#ifndef MODEL_XNODE_H
#define MODEL_XNODE_H

#include "imm/imm.h"

struct model_xnode_null
{
  struct imm_mute_state F;
  struct imm_frame_state R;
  struct imm_mute_state G;
};

struct model_xnode_alt
{
  struct imm_mute_state S;
  struct imm_frame_state N;
  struct imm_mute_state B;
  struct imm_mute_state E;
  struct imm_frame_state J;
  struct imm_frame_state C;
  struct imm_mute_state T;
};

struct model_xnode
{
  struct model_xnode_null null;
  struct model_xnode_alt alt;
};

#endif
