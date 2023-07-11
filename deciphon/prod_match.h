#ifndef DECIPHON_PROD_MATCH_H
#define DECIPHON_PROD_MATCH_H

#include "size.h"

struct dcp_prod_match
{
  long id;

  long seq_id;

  char protein[DCP_PROFILE_NAME_SIZE];
  char abc[DCP_ABC_NAME_SIZE];

  double alt;
  double null;
  double evalue;
};

void dcp_prod_match_init(struct dcp_prod_match *);
void dcp_prod_match_set_protein(struct dcp_prod_match *, char const *);
void dcp_prod_match_set_abc(struct dcp_prod_match *, char const *);
double dcp_prod_match_get_lrt(struct dcp_prod_match const *);

#endif
