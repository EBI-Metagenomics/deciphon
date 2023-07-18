#include "chararray.h"
#include "rc.h"
#include <stdlib.h>

void dcp_chararray_init(struct dcp_chararray *x)
{
  x->size = 0;
  x->capacity = 0;
  x->data = NULL;
}

void dcp_chararray_cleanup(struct dcp_chararray *x)
{
  if (x)
  {
    free(x->data);
    x->data = NULL;
    x->size = 0;
    x->capacity = 0;
  }
}

static size_t next_capacity(size_t size);

int dcp_chararray_append(struct dcp_chararray *x, char c)
{
  if (x->size + 1 > x->capacity)
  {
    char *data = realloc(x->data, next_capacity(x->capacity));
    if (!data) return DCP_ENOMEM;
    x->capacity = next_capacity(x->capacity);
    x->data = data;
  }

  x->data[x->size++] = c;

  return 0;
}

void dcp_chararray_reset(struct dcp_chararray *x) { x->size = 0; }

static size_t next_capacity(size_t x)
{
  size_t const mininc = 32;
  size_t const maxinc = 16 * 1024 * 1024;

  if (x < mininc) return x + mininc;
  if (x > maxinc) return x + maxinc;

  return 2 * x;
}
