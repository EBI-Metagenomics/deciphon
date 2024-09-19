#ifndef TRANS_H
#define TRANS_H

#include "compiler.h"

#define TRANS_SIZE 7

struct trans
{
  union
  {
    struct
    {
      float MM;
      float MI;
      float MD;
      float IM;
      float II;
      float DM;
      float DD;
    } ATTRIBUTE_PACKED;
    struct
    {
      float data[TRANS_SIZE];
    };
  };
};

#endif
