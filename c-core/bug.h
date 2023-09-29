#ifndef DECIPHON_BUG_H
#define DECIPHON_BUG_H

#include "compiler.h"
#include <assert.h>

DCP_UNUSED DCP_CONST int bug_on_reach(void)
{
  assert(0);
  return 0;
}

#endif
