#include "deciphon/fs.h"
#include "deciphon/press.h"
#include "vendor/minctest.h"

#define HMMFILE "minifam.hmm"
#define DBFILE "test_press.dcp"

static long filesize(char const *filename)
{
  long size = 0;
  eq_or_exit(fs_size(filename, &size), 0);
  return size;
}

int main(void)
{
  struct press *press = press_new();

  eq(press_setup(press, 1, 0.01), 0);
  eq_or_exit(press_open(press, HMMFILE, DBFILE), 0);

  eq(press_nproteins(press), 3);
  int rc = 0;
  while (!press_end(press))
  {
    if ((rc = press_next(press))) break;
  }
  eq(rc, 0);

  eq(press_close(press), 0);
  press_del(press);

  eq(filesize(DBFILE), 3536712);
  remove(DBFILE);

  return lfails;
}
