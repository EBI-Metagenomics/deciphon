#ifndef DATABASE_READER_H
#define DATABASE_READER_H

#include "entry_dist.h"
#include "imm/imm.h"
#include "lip/lip.h"
#include "model_params.h"

struct database_reader
{
  int num_proteins;
  uint32_t *protein_sizes;
  FILE *fp;
  struct lip_file file;

  struct imm_amino amino;
  struct imm_nuclt nuclt;
  struct imm_nuclt_code code;
  enum entry_dist entry_dist;
  float epsilon;
};

// clang-format off
void                database_reader_init(struct database_reader *);
int                 database_reader_open(struct database_reader *, char const *filename);
int                 database_reader_close(struct database_reader *);
struct model_params database_reader_params(struct database_reader const *, struct imm_gencode const *);
// clang-format on

#endif
