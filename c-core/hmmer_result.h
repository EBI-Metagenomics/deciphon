#ifndef DECIPHON_HMMER_RESULT_H
#define DECIPHON_HMMER_RESULT_H

#include <stdio.h>

struct h3client_result;

struct dcp_hmmer_result
{
  struct h3client_result *handle;
};

int dcp_hmmer_result_init(struct dcp_hmmer_result *);
void dcp_hmmer_result_cleanup(struct dcp_hmmer_result *);

int dcp_hmmer_result_nhits(struct dcp_hmmer_result const *);
double dcp_hmmer_result_evalue_ln(struct dcp_hmmer_result const *);
int dcp_hmmer_result_pack(struct dcp_hmmer_result const *, FILE *);

#endif
