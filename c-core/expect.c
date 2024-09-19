#include "expect.h"
#include "error.h"
#include "lio.h"
#include "read.h"
#include <string.h>

int expect_key(struct lio_reader *x, char const *key)
{
  uint32_t size = 0;
  int rc = read_string(x, &size);
  if (rc) return rc;

  unsigned char buf[16] = {0};
  if ((size_t)size > sizeof(buf)) return error(DCP_EFDATA);

  if (lio_readb(x, size, buf)) return error(DCP_EFREAD);
  if (size != (uint32_t)strlen(key)) return error(DCP_EFDATA);
  if (memcmp(key, buf, size) != 0) return error(DCP_EFDATA);
  return 0;
}

int expect_map(struct lio_reader *x, uint32_t size)
{
  uint32_t u32 = 0;
  int rc = read_map(x, &u32);
  if (rc) return rc;
  return u32 == size ? 0 : DCP_EFDATA;
}
