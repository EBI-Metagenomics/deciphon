#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include "aye.h"
#include "deciphon.h"
#include "fs.h"

static inline void setup_database(int gencode_id, float epsilon,
                                  char const *hmmfile, char const *dbfile)
{
  struct dcp_press *press = NULL;
  fs_rmfile(dbfile);

  aye(press = dcp_press_new());
  aye(dcp_press_setup(press, gencode_id, epsilon) == 0);
  aye(dcp_press_open(press, hmmfile, dbfile) == 0);

  while (!dcp_press_end(press))
    aye(dcp_press_next(press) == 0);

  aye(dcp_press_close(press) == 0);
  dcp_press_del(press);
}

static inline void cleanup_database(char const *dbfile) { fs_rmfile(dbfile); }

static inline long chksum(char const *filename)
{
  long chk = 0;
  aye(fs_chksum(filename, &chk) == 0);
  return chk;
}

static inline long filesize(char const *filename)
{
  long size = 0;
  aye(fs_size(filename, &size) == 0);
  return size;
}

#endif
