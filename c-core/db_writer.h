#ifndef DECIPHON_DB_WRITER_H
#define DECIPHON_DB_WRITER_H

#include "entry_dist.h"
#include "imm/imm.h"
#include "lip/lip.h"
#include "model_params.h"
#include "rc.h"
#include <stdio.h>

struct dcp_db_writer
{
  unsigned nproteins;
  unsigned header_size;
  struct lip_file file;
  struct
  {
    struct lip_file header;
    struct lip_file prot_sizes;
    struct lip_file proteins;
  } tmp;

  struct dcp_model_params params;
  struct imm_nuclt_code code;
};

struct dcp_protein;

void dcp_db_writer_init(struct dcp_db_writer *, struct dcp_model_params);
int dcp_db_writer_open(struct dcp_db_writer *, FILE *);
int dcp_db_writer_pack(struct dcp_db_writer *, struct dcp_protein const *);
int dcp_db_writer_close(struct dcp_db_writer *);

#endif