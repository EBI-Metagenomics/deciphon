#include "product_line.h"
#include "array_size_field.h"
#include "error.h"
#include "sizeof_field.h"
#include "xstrcpy.h"
#include <string.h>

void product_line_init(struct product_line *x)
{
  x->sequence = 0;

  x->window = 0;
  x->window_start = 0;
  x->window_stop = 0;

  x->hit = 0;
  x->hit_start = 0;
  x->hit_stop = 0;

  memset(x->protein, 0, sizeof_field(struct product_line, protein));
  memset(x->abc, 0, sizeof_field(struct product_line, abc));

  x->lrt = 0;
  x->logevalue = 1;
}

int product_line_set_accession(struct product_line *x, char const *accession)
{
  size_t size = array_size_field(struct product_line, protein);
  return xstrcpy(x->protein, accession, size) ? error(DCP_ELONGACCESSION) : 0;
}

int product_line_set_abc(struct product_line *x, char const *abc)
{
  size_t size = array_size_field(struct product_line, abc);
  return xstrcpy(x->abc, abc, size) ? error(DCP_ELONGABC) : 0;
}
