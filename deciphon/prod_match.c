#include "prod_match.h"
#include "lrt.h"
#include "sizeof_field.h"
#include <string.h>

void dcp_prod_match_init(struct dcp_prod_match *x)
{
  x->id = 0;
  x->seq_id = 0;

  memset(x->protein, 0, sizeof_field(struct dcp_prod_match, protein));
  memset(x->abc, 0, sizeof_field(struct dcp_prod_match, abc));

  x->alt = 0;
  x->null = 0;
  x->evalue = 0;
}

void dcp_prod_match_set_protein(struct dcp_prod_match *x, char const *protein)
{
  strcpy(x->protein, protein);
}

void dcp_prod_match_set_abc(struct dcp_prod_match *x, char const *abc)
{
  strcpy(x->abc, abc);
}

double dcp_prod_match_get_lrt(struct dcp_prod_match const *x)
{
  return lrt(x->null, x->alt);
}
