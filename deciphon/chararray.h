#ifndef DECIPHON_CHARARRAY_H
#define DECIPHON_CHARARRAY_H

#include <stddef.h>

struct dcp_chararray
{
  size_t size;
  size_t capacity;
  char *data;
};

void dcp_chararray_init(struct dcp_chararray *);
void dcp_chararray_cleanup(struct dcp_chararray *);

int dcp_chararray_append(struct dcp_chararray *, char);
void dcp_chararray_reset(struct dcp_chararray *);

#endif
