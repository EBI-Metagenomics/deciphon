#ifndef DECIPHON_DB_WRITER_H
#define DECIPHON_DB_WRITER_H

#include "entry_dist.h"
#include "lip/lip.h"
#include "protein.h"
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

  struct imm_amino amino;
  struct imm_nuclt nuclt;
  struct imm_nuclt_code code;
  enum entry_dist entry_dist;
  float epsilon;
};

int dcp_db_writer_open(struct dcp_db_writer *db, FILE *fp,
                       struct imm_amino const *amino,
                       struct imm_nuclt const *nuclt, enum entry_dist,
                       float epsilon);

int dcp_db_writer_pack(struct dcp_db_writer *db, struct dcp_protein const *);

int dcp_db_writer_close(struct dcp_db_writer *db);

#endif
