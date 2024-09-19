#include "fs.h"
#include "press.h"
#include "test_utils.h"
#include "aye.h"

#define HMMFILE "minifam.hmm"
#define DBFILE "press.dcp"

int main(void)
{
  aye_begin();
  struct press *press = NULL;
  fs_rmfile(DBFILE);

  aye(press = press_new());
  aye(press_setup(press, 1, 0.01) == 0);
  aye(press_open(press, HMMFILE, DBFILE) == 0);

  aye(press_nproteins(press) == 3);
  while (!press_end(press))
    aye(press_next(press) == 0);
  //
  // aye(press_close(press) == 0);
  // press_del(press);
  //
  // aye(filesize(DBFILE) == 3609858);

  fs_rmfile(DBFILE);
  return aye_end();
}
