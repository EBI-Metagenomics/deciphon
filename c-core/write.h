#ifndef WRITE_H
#define WRITE_H

#include "imm/imm.h"
#include "lip/1darray/1darray.h"
#include "lip/lip.h"
#include "rc.h"

static inline int write_key(struct lip_file *stream, char const key[])
{
  return lip_write_cstr(stream, key) ? 0 : DCP_EFWRITE;
}

static inline int write_mapsize(struct lip_file *stream, unsigned size)
{
  return lip_write_map_size(stream, size) ? 0 : DCP_EFWRITE;
}

#define write_int(stream, value)                                               \
  (lip_write_int(stream, value) ? 0 : DCP_EFWRITE)

#define write_float(stream, value)                                             \
  (lip_write_float(stream, value) ? 0 : DCP_EFWRITE)

static inline int write_abc(struct lip_file *stream, struct imm_abc const *abc)
{
  return imm_abc_pack(abc, stream) ? DCP_EFWRITE : 0;
}

static inline int write_f32array(struct lip_file *stream, unsigned size,
                                 float const array[])
{
  if (!lip_write_1darray_size_type(stream, size, LIP_1DARRAY_F32))
    return DCP_EFWRITE;
  return lip_write_1darray_f32_data(stream, size, array) ? 0 : DCP_EFWRITE;
}

#endif
