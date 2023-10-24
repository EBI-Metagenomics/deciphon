#include "thread_params.h"
#include "scan_params.h"
#include <stddef.h>

void thread_params_init(struct thread_params *x, struct scan_params *y,
                        struct hmmer_dialer *dialer,
                        struct protein_reader *protein,
                        struct product_thread *product_thread, int partition)
{
  x->reader = protein;
  x->partition = partition;
  x->product_thread = product_thread;
  x->dialer = dialer;
  x->multi_hits = y->multi_hits;
  x->hmmer3_compat = y->hmmer3_compat;
}
