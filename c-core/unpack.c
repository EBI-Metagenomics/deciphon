#include "unpack.h"
#include "array_size.h"
#include "error.h"
#include "imm/abc.h"
#include "lip/1darray/1darray.h"
#include "lip/file/read_cstr.h"
#include "lip/file/read_map.h"
#include "lip/file/read_str.h"
#include <string.h>

int unpack_mapsize(struct lip_file *file, unsigned size)
{
  unsigned sz = 0;
  if (!lip_read_map_size(file, &sz)) return error(DCP_EFREAD);
  return size == sz ? 0 : error(DCP_EFDATA);
}

int unpack_key(struct lip_file *file, char const key[])
{
  unsigned size = 0;
  char buf[32] = {0};

  if (!lip_read_str_size(file, &size)) return error(DCP_EFREAD);

  if (size > array_size(buf)) return error(DCP_EFDATA);

  if (!lip_read_str_data(file, size, buf)) return error(DCP_EFREAD);

  if (size != (unsigned)strlen(key)) return error(DCP_EFDATA);

  return strncmp(key, buf, size) ? error(DCP_EFDATA) : 0;
}

int unpack_abc(struct lip_file *file, struct imm_abc *abc)
{
  return imm_abc_unpack(abc, file) ? error(DCP_EFREAD) : 0;
}

int unpack_f32array(struct lip_file *file, unsigned size, float array[])
{
  unsigned sz = 0;
  enum lip_1darray_type type = 0;
  if (!lip_read_1darray_size_type(file, &sz, &type)) return error(DCP_EFREAD);
  if (size != sz) return error(DCP_EFREAD);
  if (type != LIP_1DARRAY_F32) return error(DCP_EFREAD);
  return lip_read_1darray_f32_data(file, size, array) ? 0 : error(DCP_EFREAD);
}

int read_str(struct lip_file *file, unsigned size, char str[])
{
  return lip_read_cstr(file, size, str) ? 0 : error(DCP_EFREAD);
}
