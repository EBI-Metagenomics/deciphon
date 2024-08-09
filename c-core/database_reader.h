#ifndef DATABASE_READER_H
#define DATABASE_READER_H

#include "entry_dist.h"
#include "imm_amino.h"
#include "imm_nuclt.h"
#include "imm_nuclt_code.h"
#include "lite_pack_io.h"
#include "model_params.h"
#include <stdint.h>

struct database_reader
{
  int num_proteins;
  uint32_t *protein_sizes;
  struct lio_reader file;

  struct imm_amino amino;
  struct imm_nuclt nuclt;
  struct imm_nuclt_code code;
  enum entry_dist entry_dist;
  float epsilon;
  bool has_ga;
};

void                database_reader_init(struct database_reader *);
int                 database_reader_open(struct database_reader *, char const *filename);
int                 database_reader_close(struct database_reader *);
struct model_params database_reader_params(struct database_reader const *, struct imm_gencode const *);

#endif
