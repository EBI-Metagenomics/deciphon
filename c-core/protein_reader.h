#ifndef PROTEIN_READER_H
#define PROTEIN_READER_H

#include "deciphon_limits.h"

struct db_reader;
struct protein_iter;

struct protein_reader
{
  int num_partitions;
  int size_cumsum[DCP_NPARTITIONS_MAX + 1];
  long offset[DCP_NPARTITIONS_MAX + 1];
  struct db_reader *db;
};

struct imm_gencode;

// clang-format off
void protein_reader_init(struct protein_reader *);
int  protein_reader_setup(struct protein_reader *, struct db_reader *, int num_partitions);
int  protein_reader_num_partitions(struct protein_reader const *);
int  protein_reader_partition_size(struct protein_reader const *, int partition);
int  protein_reader_size(struct protein_reader const *);
int  protein_reader_iter(struct protein_reader *, int partition, struct protein_iter *);
// clang-format on

#endif
