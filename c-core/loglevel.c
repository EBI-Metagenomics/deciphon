#include "loglevel.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static _Thread_local int level   = LOGLEVEL_ERROR;
static _Thread_local bool cached = false;

int loglevel(void)
{
  if (cached) return level;
  char const *x = getenv("DECIPHON_LOGLEVEL");
  if (x) level = atoi(x);
  cached = true;
  return level;
}
