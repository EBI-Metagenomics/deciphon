#include "write.h"
#include "deciphon.h"
#include "error.h"
#include "imm_abc.h"
#include "lio.h"
#include <string.h>

int write_cstring(struct lio_writer *x, char const *string)
{
  uint32_t length = (uint32_t)strlen(string);
  int rc = 0;
  if ((rc = lio_write(x, lip_pack_string(lio_alloc(x), length)))) return error_system(DCP_EFWRITE, lio_syserror(rc));
  if ((rc = lio_writeb(x, length, string)))                       return error_system(DCP_EFWRITE, lio_syserror(rc));
  return 0;
}

int write_map(struct lio_writer *x, int size)
{
  int rc = 0;
  if ((rc = lio_write(x, lip_pack_map(lio_alloc(x), size)))) return error_system(DCP_EFWRITE, lio_syserror(rc));
  return 0;
}

int write_array(struct lio_writer *x, int size)
{
  int rc = 0;
  if ((rc = lio_write(x, lip_pack_array(lio_alloc(x), size)))) return error_system(DCP_EFWRITE, lio_syserror(rc));
  return 0;
}

int write_i(struct lio_writer *x, int data)
{
  int rc = 0;
  if ((rc = lio_write(x, lip_pack_int(lio_alloc(x), data)))) return error_system(DCP_EFWRITE, lio_syserror(rc));
  return 0;
}

int write_u(struct lio_writer *x, unsigned data)
{
  int rc = 0;
  if ((rc = lio_write(x, lip_pack_int(lio_alloc(x), data)))) return error_system(DCP_EFWRITE, lio_syserror(rc));
  return 0;
}

int write_float(struct lio_writer *x, float data)
{
  int rc = 0;
  if ((rc = lio_write(x, lip_pack_float(lio_alloc(x), data)))) return error_system(DCP_EFWRITE, lio_syserror(rc));
  return 0;
}

int write_bool(struct lio_writer *x, bool data)
{
  int rc = 0;
  if ((rc = lio_write(x, lip_pack_bool(lio_alloc(x), data)))) return error_system(DCP_EFWRITE, lio_syserror(rc));
  return 0;
}

int write_f32array(struct lio_writer *x, uint32_t size, float const *array)
{
  uint32_t n = size * sizeof(float);
  int rc = 0;
  if ((rc = lio_write(x, lip_pack_bin(lio_alloc(x), n)))) return error_system(DCP_EFWRITE, lio_syserror(rc));
  if ((rc = lio_writeb(x, size * sizeof(float), array)))  return error_system(DCP_EFWRITE, lio_syserror(rc));
  return 0;
}
