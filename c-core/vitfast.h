#ifndef VITFAST_H
#define VITFAST_H

#include "viterbi_ids.h"

#define VITFAST_TABLE_SIZE 1364
#define VITFAST_TIME_FRAME 6

struct vitfast;

typedef int (*vitfast_code_fn)(int pos, int len, void *arg);

struct vitfast *vitfast_new(void);
int             vitfast_setup(struct vitfast *, int K);
void            vitfast_set_extr_trans(struct vitfast *, enum extr_trans_id, float scalar);
void            vitfast_set_core_trans(struct vitfast *, enum core_trans_id, float scalar, int k);
void            vitfast_set_null(struct vitfast *, float scalar, int code);
void            vitfast_set_background(struct vitfast *, float scalar, int code);
void            vitfast_set_match(struct vitfast *, float scalar, int k, int code);
float           vitfast_cost(struct vitfast *, int L, vitfast_code_fn, void *code_arg);
int             vitfast_num_packs(int K);
void            vitfast_del(struct vitfast const *);

#endif
