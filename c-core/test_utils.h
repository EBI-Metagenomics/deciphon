#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include "aye.h"
#include "fs.h"
#include "press.h"

static inline void setup_database(int gencode_id, float epsilon,
                                  char const *hmmfile, char const *dbfile)
{
  struct press *press = NULL;
  fs_rmfile(dbfile);

  aye(press = press_new());
  aye(press_setup(press, gencode_id, epsilon) == 0);
  aye(press_open(press, hmmfile, dbfile) == 0);

  while (!press_end(press))
    aye(press_next(press) == 0);

  aye(press_close(press) == 0);
  press_del(press);
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
