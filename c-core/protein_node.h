#ifndef PROTEIN_NODE_H
#define PROTEIN_NODE_H

#include "nuclt_dist.h"
#include "trans.h"

struct protein_node
{
  struct nuclt_dist nuclt_dist;
  struct trans trans;
  float *emission;
};

#endif
