#include "prod_match.h"
#include "array_size_field.h"
#include "lrt.h"
#include "sizeof_field.h"
#include "strkcpy.h"
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
  size_t size = array_size_field(struct dcp_prod_match, protein);
  strkcpy(x->protein, protein, size);
}

void dcp_prod_match_set_abc(struct dcp_prod_match *x, char const *abc)
{
  size_t size = array_size_field(struct dcp_prod_match, abc);
  strkcpy(x->abc, abc, size);
}

float dcp_prod_match_get_lrt(struct dcp_prod_match const *x)
{
  return lrt(x->null, x->alt);
}
