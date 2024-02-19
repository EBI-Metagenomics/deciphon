#include "env.h"
#include <stdlib.h>
#include <string.h>

static _Thread_local char last_name[128] = {0};
static _Thread_local char const *last_value = NULL;

static char const *cached_getenv(char const *name)
{
  if (strlen(last_name) == strlen(name) && !strcmp(last_name, name))
    return last_value;

  last_value = getenv(name);
  last_name[0] = '\0';

  if (strlen(name) < sizeof(last_name)) strcpy(last_name, name);

  return last_value;
}

bool env_as_bool(char const *name)
{
  char const *value = cached_getenv(name);

  // false when undefined or zero;
  // true otherwise.
  return value && strcmp(value, "0");
}
