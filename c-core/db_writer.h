#ifndef DECIPHON_DB_WRITER_H
#define DECIPHON_DB_WRITER_H

#include "imm/imm.h"
#include "lip/lip.h"
#include "model_params.h"
#include "size.h"
#include <stdio.h>

struct dcp_db_writer
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

  struct dcp_model_params params;
  struct imm_nuclt_code code;
};

struct dcp_protein;

void dcp_db_writer_init(struct dcp_db_writer *, struct dcp_model_params);
int dcp_db_writer_open(struct dcp_db_writer *, FILE *restrict);
int dcp_db_writer_pack(struct dcp_db_writer *, struct dcp_protein const *);
int dcp_db_writer_close(struct dcp_db_writer *);

#endif
