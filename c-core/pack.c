#include "pack.h"
#include "error.h"
#include "imm/abc.h"
#include "lip/1darray/1darray.h"
#include "lip/file/write_cstr.h"
#include "lip/file/write_map.h"

int pack_key(struct lip_file *file, char const key[])
{
  return lip_write_cstr(file, key) ? 0 : error(DCP_EFWRITE);
}

int pack_mapsize(struct lip_file *file, unsigned size)
{
  return lip_write_map_size(file, size) ? 0 : error(DCP_EFWRITE);
}

int pack_abc(struct lip_file *file, struct imm_abc const *abc)
{
  return imm_abc_pack(abc, file) ? error(DCP_EFWRITE) : 0;
}

int pack_f32array(struct lip_file *file, unsigned size, float const array[])
{
  if (!lip_write_1darray_size_type(file, size, LIP_1DARRAY_F32))
    return error(DCP_EFWRITE);
  return lip_write_1darray_f32_data(file, size, array) ? 0 : error(DCP_EFWRITE);
}
