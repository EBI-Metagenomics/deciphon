#ifndef VIT_H
#define VIT_H

#include "vith.h"

struct trellis;

struct vit;

struct vit *vit_new(void);
void        vit_del(struct vit const *);

int   vit_setup(struct vit *, int K);
void  vit_set_extr_trans(struct vit *, enum extr_trans_id, float scalar);
void  vit_set_core_trans(struct vit *, enum core_trans_id, float scalar, int k);
void  vit_set_null(struct vit *, float scalar, int code);
void  vit_set_background(struct vit *, float scalar, int code);
void  vit_set_match(struct vit *, float scalar, int k, int code);
float vit_null(struct vit *, int L, viterbi_code_fn, void *);
float vit_cost(struct vit *, int L, viterbi_code_fn, void *);
int   vit_path(struct vit *, int L, viterbi_code_fn, void *);

struct trellis *vit_trellis(struct vit *);

#endif
