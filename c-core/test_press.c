#include "fs.h"
#include "press.h"
#include "test_utils.h"
#include "vendor/minctest.h"

#define HMMFILE "minifam.hmm"
#define DBFILE "test_press.dcp"

int main(void)
{
  struct press *press = NULL;
  fs_rmfile(DBFILE);

  ok(press = press_new());
  eq(press_setup(press, 1, 0.01), 0);
  eq(press_open(press, HMMFILE, DBFILE), 0);

  eq(press_nproteins(press), 3);
  while (!press_end(press))
    eq(press_next(press), 0);

  eq(press_close(press), 0);
  press_del(press);

  eq(filesize(DBFILE), 3536729);

  fs_rmfile(DBFILE);
  return lfails;
}
