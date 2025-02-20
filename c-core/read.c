#include "read.h"
#include "deciphon.h"
#include "error.h"
#include "imm_abc.h"
#include "lio.h"
#include <string.h>

int read_string(struct lio_reader *x, uint32_t *size)
{
  int rc = 0;
  unsigned char *buf = NULL;
  if ((rc = lio_read(x, &buf)))
  {
    return lio_eof(x) ? error(DCP_EENDOFFILE)
                      : error_system(DCP_EFREAD, lio_syserror(rc));
  }
  if (lio_free(x, lip_unpack_string(buf, size))) return error(DCP_EFDATA);
  return 0;
}

int read_cstring(struct lio_reader *x, uint32_t size, char *string)
{
  int rc = 0;
  uint32_t n = 0;
  unsigned char *buf = NULL;
  if ((rc = lio_read(x, &buf)))
  {
    return lio_eof(x) ? error(DCP_EENDOFFILE)
                      : error_system(DCP_EFREAD, lio_syserror(rc));
  }
  if (lio_free(x, lip_unpack_string(buf, &n)))         return error(DCP_EFDATA);
  if (n >= size)                                       return error(DCP_EFDATA);
  if ((rc = lio_readb(x, n, (unsigned char *)string))) return error_system(DCP_EFREAD, lio_syserror(rc));
  string[n] = '\0';
  return 0;
}

int read_map(struct lio_reader *x, uint32_t *size)
{
  int rc = 0;
  unsigned char *buf = NULL;
  if ((rc = lio_read(x, &buf)))
  {
    return lio_eof(x) ? error(DCP_EENDOFFILE)
                      : error_system(DCP_EFREAD, lio_syserror(rc));
  }
  if (lio_free(x, lip_unpack_map(buf, size))) return error(DCP_EFDATA);
  return 0;
}

int read_array(struct lio_reader *x, uint32_t *size)
{
  int rc = 0;
  unsigned char *buf = NULL;
  if ((rc = lio_read(x, &buf)))
  {
    return lio_eof(x) ? error(DCP_EENDOFFILE)
                      : error_system(DCP_EFREAD, lio_syserror(rc));
  }
  if (lio_free(x, lip_unpack_array(buf, size))) return error(DCP_EFDATA);
  return 0;
}

int read_i32(struct lio_reader *x, int32_t *data)
{
  int rc = 0;
  unsigned char *buf = NULL;
  if ((rc = lio_read(x, &buf)))
  {
    return lio_eof(x) ? error(DCP_EENDOFFILE)
                      : error_system(DCP_EFREAD, lio_syserror(rc));
  }
  if (lio_free(x, lip_unpack_int(buf, data))) return error(DCP_EFDATA);
  return 0;
}

int read_u32(struct lio_reader *x, uint32_t *data)
{
  int rc = 0;
  unsigned char *buf = NULL;
  if ((rc = lio_read(x, &buf)))
  {
    return lio_eof(x) ? error(DCP_EENDOFFILE)
                      : error_system(DCP_EFREAD, lio_syserror(rc));
  }
  if (lio_free(x, lip_unpack_int(buf, data))) return error(DCP_EFDATA);
  return 0;
}

int read_bool(struct lio_reader *x, bool *data)
{
  int rc = 0;
  unsigned char *buf = NULL;
  if ((rc = lio_read(x, &buf)))
  {
    return lio_eof(x) ? error(DCP_EENDOFFILE)
                      : error_system(DCP_EFREAD, lio_syserror(rc));
  }
  if (lio_free(x, lip_unpack_bool(buf, data))) return error(DCP_EFDATA);
  return 0;
}

int read_float(struct lio_reader *x, float *data)
{
  int rc = 0;
  unsigned char *buf = NULL;
  if ((rc = lio_read(x, &buf)))
  {
    return lio_eof(x) ? error(DCP_EENDOFFILE)
                      : error_system(DCP_EFREAD, lio_syserror(rc));
  }
  if (lio_free(x, lip_unpack_float(buf, data))) return error(DCP_EFDATA);
  return 0;
}

int read_f32array(struct lio_reader *x, uint32_t size, float *array)
{
  int rc = 0;
  uint32_t n = 0;
  unsigned char *buf = NULL;
  if ((rc = lio_read(x, &buf)))
  {
    return lio_eof(x) ? error(DCP_EENDOFFILE)
                      : error_system(DCP_EFREAD, lio_syserror(rc));
  }
  if (lio_free(x, lip_unpack_bin(buf, &n)))           return error(DCP_EFDATA);
  if (n != size * sizeof(float))                      return error(DCP_EFDATA);
  if ((rc = lio_readb(x, n, (unsigned char *)array))) return error_system(DCP_EFREAD, lio_syserror(rc));
  return 0;
}
