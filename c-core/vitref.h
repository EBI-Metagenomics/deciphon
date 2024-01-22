#ifndef VITREF_H
#define VITREF_H

#include "viterbi_ids.h"

#define VITREF_TABLE_SIZE 1364
#define VITREF_TIME_FRAME 6

struct vitref;

typedef int (*vitref_code_fn)(int pos, int len, void *arg);

struct vitref *vitref_new(void);
int            vitref_setup(struct vitref *, int K);
float          vitref_get_extr_trans(struct vitref const *, enum extr_trans_id);
float          vitref_get_core_trans(struct vitref const *, enum core_trans_id, int k);
float          vitref_get_null(struct vitref const *, int code);
float          vitref_get_background(struct vitref const *x, int code);
float          vitref_get_match(struct vitref const *x, int k, int code);
float          vitref_cost(struct vitref *, int L, vitref_code_fn, void *code_arg);
void           vitref_del(struct vitref const *);
void           vitref_sample(struct vitref *, int seed);

#endif
