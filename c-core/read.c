#include "read.h"
#include "error.h"
#include "imm_abc.h"
#include "lio.h"
#include <string.h>

int read_string(struct lio_reader *x, uint32_t *size)
{
  unsigned char *ptr = NULL;
  if (!(ptr = lio_read(x))) return DCP_EFREAD;
  if (lio_free(x, lip_unpack_string(ptr, size))) return DCP_EFDATA;
  return 0;
}

int read_cstring(struct lio_reader *x, uint32_t size, char *string)
{
  uint32_t u32 = 0;
  int rc = read_string(x, &u32);
  if (rc) return rc;

  if ((size_t)u32 >= size) return DCP_EFDATA;

  if (lio_readb(x, u32, (unsigned char *)string)) return DCP_EFREAD;
  string[u32] = '\0';
  return 0;
}

int read_map(struct lio_reader *x, uint32_t *size)
{
  unsigned char *ptr = NULL;
  if (!(ptr = lio_read(x))) return DCP_EFREAD;
  if (lio_free(x, lip_unpack_map(ptr, size))) return DCP_EFDATA;
  return 0;
}

int read_array(struct lio_reader *x, uint32_t *size)
{
  unsigned char *ptr = NULL;
  if (!(ptr = lio_read(x))) return DCP_EFREAD;
  if (lio_free(x, lip_unpack_array(ptr, size))) return DCP_EFDATA;
  return 0;
}

int read_i32(struct lio_reader *x, int32_t *data)
{
  unsigned char *ptr = NULL;
  if (!(ptr = lio_read(x))) return DCP_EFREAD;
  if (lio_free(x, lip_unpack_int(ptr, data))) return DCP_EFDATA;
  return 0;
}

int read_u32(struct lio_reader *x, uint32_t *data)
{
  unsigned char *ptr = NULL;
  if (!(ptr = lio_read(x))) return DCP_EFREAD;
  if (lio_free(x, lip_unpack_int(ptr, data))) return DCP_EFDATA;
  return 0;
}

int read_bool(struct lio_reader *x, bool *data)
{
  unsigned char *ptr = NULL;
  if (!(ptr = lio_read(x))) return DCP_EFREAD;
  if (lio_free(x, lip_unpack_bool(ptr, data))) return DCP_EFDATA;
  return 0;
}

int read_float(struct lio_reader *x, float *data)
{
  unsigned char *ptr = NULL;
  if (!(ptr = lio_read(x))) return DCP_EFREAD;
  if (lio_free(x, lip_unpack_float(ptr, data))) return DCP_EFDATA;
  return 0;
}

int read_f32array(struct lio_reader *x, uint32_t size, float *array)
{
  unsigned char *ptr = NULL;
  uint32_t nbytes = 0;
  if (!(ptr = lio_read(x))) return DCP_EFREAD;
  if (lio_free(x, lip_unpack_bin(ptr, &nbytes))) return DCP_EFDATA;
  if (nbytes != size * sizeof(float)) return DCP_EFDATA;
  if (lio_readb(x, nbytes, (unsigned char *)array)) return DCP_EFREAD;
  return 0;
}
