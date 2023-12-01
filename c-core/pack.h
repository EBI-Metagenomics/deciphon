#ifndef PACK_H
#define PACK_H

#include "error.h"
#include "imm/abc.h"
#include "lip/1darray/1darray.h"
#include "lip/file/write_cstr.h"
#include "lip/file/write_map.h"
#include "rc.h"

static inline int pack_key(struct lip_file *stream, char const key[])
{
  return lip_write_cstr(stream, key) ? 0 : error(DCP_EFWRITE);
}

static inline int pack_mapsize(struct lip_file *stream, unsigned size)
{
  return lip_write_map_size(stream, size) ? 0 : error(DCP_EFWRITE);
}

#define pack_int(stream, value)                                                \
  (lip_write_int(stream, value) ? 0 : error(DCP_EFWRITE))

#define pack_float(stream, value)                                              \
  (lip_write_float(stream, value) ? 0 : error(DCP_EFWRITE))

#define pack_bool(stream, value)                                               \
  (lip_write_bool(stream, value) ? 0 : error(DCP_EFWRITE))

static inline int pack_abc(struct lip_file *stream, struct imm_abc const *abc)
{
  return imm_abc_pack(abc, stream) ? error(DCP_EFWRITE) : 0;
}

static inline int pack_f32array(struct lip_file *stream, unsigned size,
                                float const array[])
{
  if (!lip_write_1darray_size_type(stream, size, LIP_1DARRAY_F32))
    return error(DCP_EFWRITE);
  return lip_write_1darray_f32_data(stream, size, array) ? 0
                                                         : error(DCP_EFWRITE);
}

#endif
