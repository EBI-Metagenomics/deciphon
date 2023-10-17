#ifndef PRODUCT_LINE_H
#define PRODUCT_LINE_H

#include "size.h"

struct product_line
{
  long sequence;

  int window;
  int window_start;
  int window_stop;

  char protein[DCP_PROFILE_NAME_SIZE];
  char abc[DCP_ABC_NAME_SIZE];

  float lrt;
  float evalue;
};

// clang-format off
void product_line_init(struct product_line *);
int  product_line_set_protein(struct product_line *, char const *);
int  product_line_set_abc(struct product_line *, char const *);
// clang-format on

#endif
