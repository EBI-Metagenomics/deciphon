#ifndef DECIPHON_READ_H
#define DECIPHON_READ_H

#include "array_size.h"
#include "imm/imm.h"
#include "lip/1darray/1darray.h"
#include "lip/lip.h"
#include "rc.h"
#include <string.h>

static inline int read_mapsize(struct lip_file *file, unsigned size)
{
  unsigned sz = 0;
  if (!lip_read_map_size(file, &sz)) return DCP_EFREAD;
  return size == sz ? 0 : DCP_EFDATA;
}

static inline int read_key(struct lip_file *file, char const key[])
{
  unsigned size = 0;
  char buf[32] = {0};

  if (!lip_read_str_size(file, &size)) return DCP_EFREAD;

  if (size > array_size(buf)) return DCP_EFDATA;

  if (!lip_read_str_data(file, size, buf)) return DCP_EFREAD;

  if (size != (unsigned)strlen(key)) return DCP_EFDATA;

  return strncmp(key, buf, size) ? DCP_EFDATA : 0;
}

#define read_int(stream, ptr) (lip_read_int(stream, ptr) ? 0 : DCP_EFREAD)
#define read_float(stream, ptr) (lip_read_float(stream, ptr) ? 0 : DCP_EFREAD)

static inline int read_abc(struct lip_file *stream, struct imm_abc *abc)
{
  return imm_abc_unpack(abc, stream) ? DCP_EFREAD : 0;
}

static inline int read_f32array(struct lip_file *stream, unsigned size,
                                float array[])
{
  unsigned sz = 0;
  enum lip_1darray_type type = 0;
  if (!lip_read_1darray_size_type(stream, &sz, &type)) return DCP_EFREAD;
  if (size != sz) return DCP_EFREAD;
  if (type != LIP_1DARRAY_F32) return DCP_EFREAD;
  return lip_read_1darray_f32_data(stream, size, array) ? 0 : DCP_EFREAD;
}

#endif
