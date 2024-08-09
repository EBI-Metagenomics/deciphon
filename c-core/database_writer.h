#ifndef DATABASE_WRITER_H
#define DATABASE_WRITER_H

#include "fs.h"
#include "imm_nuclt_code.h"
#include "lite_pack_io.h"
#include "model_params.h"

struct database_writer
{
  int nproteins;
  struct lio_writer file;
  struct
  {
    char header_name[FS_PATH_MAX];
    char sizes_name[FS_PATH_MAX];
    char proteins_name[FS_PATH_MAX];
    struct lio_writer header;
    struct lio_writer sizes;
    struct lio_writer proteins;
  } tmp;

  struct model_params params;
  struct imm_nuclt_code code;
  bool has_ga;
};

struct protein;

void database_writer_init(struct database_writer *, struct model_params);
int  database_writer_open(struct database_writer *, int);
int  database_writer_pack(struct database_writer *, struct protein const *);
void database_writer_set_has_ga(struct database_writer *, bool has_ga);
int  database_writer_close(struct database_writer *);

#endif
