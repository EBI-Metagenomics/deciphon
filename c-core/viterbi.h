#ifndef VITERBI_H
#define VITERBI_H

enum extr_trans_id
{
  EXTR_TRANS_RR,
  EXTR_TRANS_SN,
  EXTR_TRANS_NN,
  EXTR_TRANS_SB,
  EXTR_TRANS_NB,
  EXTR_TRANS_EB,
  EXTR_TRANS_JB,
  EXTR_TRANS_EJ,
  EXTR_TRANS_JJ,
  EXTR_TRANS_EC,
  EXTR_TRANS_CC,
  EXTR_TRANS_ET,
  EXTR_TRANS_CT,
};

enum core_trans_id
{
  CORE_TRANS_BM,
  CORE_TRANS_MM,
  CORE_TRANS_MI,
  CORE_TRANS_MD,
  CORE_TRANS_IM,
  CORE_TRANS_II,
  CORE_TRANS_DM,
  CORE_TRANS_DD,
};

typedef int (*viterbi_code_fn)(int pos, int len, void *arg);

struct trellis;
struct viterbi;

struct viterbi *viterbi_new(void);
void            viterbi_del(struct viterbi const *);

int   viterbi_setup(struct viterbi *, int K);
void  viterbi_set_extr_trans(struct viterbi *, enum extr_trans_id, float scalar);
void  viterbi_set_core_trans(struct viterbi *, enum core_trans_id, float scalar, int k);
void  viterbi_set_null(struct viterbi *, float scalar, int code);
void  viterbi_set_background(struct viterbi *, float scalar, int code);
void  viterbi_set_match(struct viterbi *, float scalar, int k, int code);
float viterbi_null(struct viterbi *, int L, viterbi_code_fn, void *);
float viterbi_cost(struct viterbi *, int L, viterbi_code_fn, void *);
int   viterbi_path(struct viterbi *, int L, viterbi_code_fn, void *);

struct trellis *viterbi_trellis(struct viterbi *);
int             viterbi_table_size(void);

#endif
