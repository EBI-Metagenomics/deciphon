#ifndef DECIPHON_NODE_H
#define DECIPHON_NODE_H

#include "imm/imm.h"
#include "nuclt_dist.h"

#ifdef I
#undef I
#endif

struct dcp_model_node
{
  union
  {
    struct imm_frame_state M;
    struct
    {
      struct imm_frame_state state;
      struct dcp_nuclt_dist nucltd;
    } match;
  };
  struct imm_frame_state I;
  struct imm_mute_state D;
};

#endif
