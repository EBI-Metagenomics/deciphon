#ifndef PROTEIN_READER_H
#define PROTEIN_READER_H

#define PROTEIN_READER_MAX_PARTITIONS 128

struct database_reader;
struct protein_iter;

struct protein_reader
{
  int num_partitions;
  int size_cumsum[PROTEIN_READER_MAX_PARTITIONS + 1];
  long offset[PROTEIN_READER_MAX_PARTITIONS + 1];
  int file_descriptor;
};

struct imm_gencode;

void protein_reader_init(struct protein_reader *);
int  protein_reader_setup(struct protein_reader *, struct database_reader *, int num_partitions);
int  protein_reader_num_partitions(struct protein_reader const *);
int  protein_reader_partition_size(struct protein_reader const *, int partition);
int  protein_reader_partition_cumsize(struct protein_reader const *, int partition);
int  protein_reader_size(struct protein_reader const *);
int  protein_reader_iter(struct protein_reader *, int partition, struct protein_iter *);
void protein_reader_cleanup(struct protein_reader *);

#endif
