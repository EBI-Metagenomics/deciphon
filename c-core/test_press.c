#include "deciphon/fs.h"
#include "deciphon/press.h"
#include "vendor/minctest.h"

#define HMMFILE "minifam.hmm"
#define DBFILE "test_press.dcp"

static long filesize(char const *filename)
{
  long size = 0;
  eq_or_exit(dcp_fs_size(filename, &size), 0);
  return size;
}

int main(void)
{
  struct dcp_press *press = dcp_press_new();

  eq(dcp_press_setup(press, 1, 0.01), 0);
  eq_or_exit(dcp_press_open(press, HMMFILE, DBFILE), 0);

  eq(dcp_press_nproteins(press), 3);
  int rc = 0;
  while (!dcp_press_end(press))
  {
    if ((rc = dcp_press_next(press))) break;
  }
  eq(rc, 0);

  eq(dcp_press_close(press), 0);
  dcp_press_del(press);

  eq(filesize(DBFILE), 3536712);
  remove(DBFILE);

  return lfails;
}
