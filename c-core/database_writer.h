#ifndef DATABASE_WRITER_H
#define DATABASE_WRITER_H

#include "imm_nuclt_code.h"
#include "lio.h"
#include "model_params.h"

#define DATABASE_WRITER_CHUNKS 32

struct database_writer
{
  int nproteins;
  struct lio_writer file;
  struct
  {
    struct lio_writer header;
    struct lio_writer sizes;
    struct lio_writer proteins[DATABASE_WRITER_CHUNKS];
    struct lio_writer *current_proteins;
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
