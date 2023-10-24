#ifndef DATABASE_WRITER_H
#define DATABASE_WRITER_H

#include "imm/imm.h"
#include "lip/lip.h"
#include "model_params.h"
#include "xlimits.h"
#include <stdio.h>

struct database_writer
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

// clang-format off
void database_writer_init(struct database_writer *, struct model_params);
int  database_writer_open(struct database_writer *, FILE *restrict);
int  database_writer_pack(struct database_writer *, struct protein const *);
int  database_writer_close(struct database_writer *);
// clang-format on

#endif
