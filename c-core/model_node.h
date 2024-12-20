#ifndef MODEL_NODE_H
#define MODEL_NODE_H

#include "imm_frame_state.h"
#include "imm_mute_state.h"
#include "nuclt_dist.h"

struct model_node
{
  union
  {
    struct imm_frame_state M;
    struct
    {
      struct imm_frame_state state;
      struct nuclt_dist nucltd;
    } match;
  };
  struct imm_frame_state I;
  struct imm_mute_state D;
};

#endif
