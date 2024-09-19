#include "loglevel.h"
#include <stdlib.h>
#include <string.h>

static _Thread_local int level = LOGLEVEL_ERROR;

int loglevel(void)
{
  char const *x = getenv("DECIPHON_LOGLEVEL");
  if (x) level = atoi(x);
  return level;
}
