#ifndef DECIPHON_DB_READER_H
#define DECIPHON_DB_READER_H

#include "entry_dist.h"
#include "imm/imm.h"
#include "lip/lip.h"
#include "model_params.h"

struct dcp_db_reader
{
  int nproteins;
  uint32_t *protein_sizes;
  struct lip_file file;

  struct imm_amino amino;
  struct imm_nuclt nuclt;
  struct imm_nuclt_code code;
  enum dcp_entry_dist entry_dist;
  float epsilon;
};

void dcp_db_reader_init(struct dcp_db_reader *);
int dcp_db_reader_open(struct dcp_db_reader *, FILE *);
void dcp_db_reader_close(struct dcp_db_reader *);

int dcp_db_reader_unpack_magic_number(struct dcp_db_reader *);
int dcp_db_reader_unpack_float_size(struct dcp_db_reader *);
int dcp_db_reader_unpack_prot_sizes(struct dcp_db_reader *);

struct dcp_model_params dcp_db_reader_params(struct dcp_db_reader const *,
                                             struct imm_gencode const *);

#endif
