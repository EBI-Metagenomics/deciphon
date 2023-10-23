#ifndef HMMER_RESULT_H
#define HMMER_RESULT_H

#include <stdio.h>

struct h3client_result;

struct hmmer_result
{
  struct h3client_result *handle;
};

// clang-format off
int   hmmer_result_init(struct hmmer_result *);
void  hmmer_result_cleanup(struct hmmer_result *);
int   hmmer_result_num_hits(struct hmmer_result const *);
float hmmer_result_evalue(struct hmmer_result const *);
int   hmmer_result_pack(struct hmmer_result const *, FILE *);
// clang-format on

#endif
