#ifndef PRODUCT_LINE_H
#define PRODUCT_LINE_H

struct product_line
{
  long sequence;

  int window;
  // [window_start, window_stop)
  int window_start;
  int window_stop;

  int hit;
  // [hit_start, hit_stop)
  int hit_start;
  int hit_stop;

  char protein[64];
  char abc[16];

  float lrt;
  double logevalue;
};

void product_line_init(struct product_line *);
int  product_line_set_protein(struct product_line *, char const *);
int  product_line_set_abc(struct product_line *, char const *);

#endif
