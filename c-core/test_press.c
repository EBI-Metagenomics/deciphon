#include "aye.h"
#include "deciphon.h"
#include "fs.h"
#include "test_utils.h"

#define HMMFILE "minifam.hmm"
#define DBFILE "press.dcp"

int main(void)
{
  aye_begin();
  struct dcp_press *press = NULL;
  fs_rmfile(DBFILE);

  aye(press = dcp_press_new());
  aye(dcp_press_setup(press, 1, 0.01) == 0);
  aye(dcp_press_open(press, HMMFILE, DBFILE) == 0);

  aye(dcp_press_nproteins(press) == 3);
  while (!dcp_press_end(press))
    aye(dcp_press_next(press) == 0);

  aye(dcp_press_close(press) == 0);
  dcp_press_del(press);

  aye(filesize(DBFILE) == 3609858);

  fs_rmfile(DBFILE);
  return aye_end();
}
