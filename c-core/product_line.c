#include "product_line.h"
#include "array_size_field.h"
#include "lrt.h"
#include "sizeof_field.h"
#include "strkcpy.h"
#include <string.h>

void product_line_init(struct product_line *x)
{
  x->sequence = 0;

  x->window = 0;
  x->window_start = 0;
  x->window_stop = 0;

  memset(x->protein, 0, sizeof_field(struct product_line, protein));
  memset(x->abc, 0, sizeof_field(struct product_line, abc));

  x->lrt = 0;
  x->evalue = 0;
}

void product_line_set_protein(struct product_line *x, char const *protein)
{
  size_t size = array_size_field(struct product_line, protein);
  strkcpy(x->protein, protein, size);
}

void product_line_set_abc(struct product_line *x, char const *abc)
{
  size_t size = array_size_field(struct product_line, abc);
  strkcpy(x->abc, abc, size);
}
