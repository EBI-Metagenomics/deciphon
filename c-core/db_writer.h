#ifndef DB_WRITER_H
#define DB_WRITER_H

#include "deciphon_limits.h"
#include "imm/imm.h"
#include "lip/lip.h"
#include "model_params.h"
#include <stdio.h>

struct db_writer
{
  int nproteins;
  struct lip_file file;
  struct
  {
    char header_name[DCP_PATH_MAX];
    char sizes_name[DCP_PATH_MAX];
    char proteins_name[DCP_PATH_MAX];
    struct lip_file header;
    struct lip_file sizes;
    struct lip_file proteins;
  } tmp;

  struct model_params params;
  struct imm_nuclt_code code;
};

struct protein;

void db_writer_init(struct db_writer *, struct model_params);
int db_writer_open(struct db_writer *, FILE *restrict);
int db_writer_pack(struct db_writer *, struct protein const *);
int db_writer_close(struct db_writer *);

#endif
