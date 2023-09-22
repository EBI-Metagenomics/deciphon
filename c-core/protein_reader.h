#ifndef DECIPHON_PROTEIN_READER_H
#define DECIPHON_PROTEIN_READER_H

#include "size.h"

struct dcp_db_reader;
struct dcp_protein_iter;

struct dcp_protein_reader
{
  int npartitions;
  int partition_csum[DCP_NPARTITIONS_MAX + 1];
  long partition_offset[DCP_NPARTITIONS_MAX + 1];
  struct dcp_db_reader *db;
};

struct imm_gencode;

void dcp_protein_reader_init(struct dcp_protein_reader *);
int dcp_protein_reader_setup(struct dcp_protein_reader *,
                             struct dcp_db_reader *, int npartitions);

int dcp_protein_reader_npartitions(struct dcp_protein_reader const *);
int dcp_protein_reader_partition_size(struct dcp_protein_reader const *,
                                      int partition);
int dcp_protein_reader_size(struct dcp_protein_reader const *);
int dcp_protein_reader_iter(struct dcp_protein_reader *, int partition,
                            struct dcp_protein_iter *);

struct dcp_model_params dcp_protein_reader_params(struct dcp_protein_reader *,
                                                  struct imm_gencode const *);

#endif
