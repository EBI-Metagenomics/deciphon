#include "bug.h"
#include "compiler_attributes.h"
#include <stdio.h>
#include <stdlib.h>

ATTRIBUTE_NORETURN void bug(char const *file, int line, char const *func)
{
  fprintf(stderr, "BUG: failure at %s:%d/%s()!\n", file, line, func);
  exit(1);
}
