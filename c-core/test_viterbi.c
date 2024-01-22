#include "idot.h"
#include "ipow.h"
#include "isum.h"
#include "sample.h"
#include "vendor/minctest.h"
#include "vitfast.h"
#include "vitref.h"

static void sample(struct vitref *ref, struct vitfast *vit, int K, int seed);
static void run(struct vitref *ref, struct vitfast *vit, char const *seq);

int main(void)
{
  struct vitref *ref = NULL;
  struct vitfast *vit = NULL;

  ok(ref = vitref_new());
  ok(vit = vitfast_new());

  for (int K = 1; K < 10; K += 3)
  {
    ok(!vitref_setup(ref, K));
    ok(!vitfast_setup(vit, K));

    for (int L = 1; L < 100; L += 11)
    {
      for (int seed = 0; seed < 100; ++seed)
      {
        vitref_sample(ref, seed);
        sample(ref, vit, K, seed);
        run(ref, vit, sample_sequence(L, "ACGT"));
      }
    }
  }

  vitref_del(ref);
  vitfast_del(vit);
}

static void sample(struct vitref *ref, struct vitfast *vit, int K, int seed)
{
  vitref_sample(ref, seed);
  vitfast_set_extr_trans(vit, EXTR_TRANS_SN, vitref_get_extr_trans(ref, EXTR_TRANS_SN));
  vitfast_set_extr_trans(vit, EXTR_TRANS_NN, vitref_get_extr_trans(ref, EXTR_TRANS_NN));
  vitfast_set_extr_trans(vit, EXTR_TRANS_SB, vitref_get_extr_trans(ref, EXTR_TRANS_SB));
  vitfast_set_extr_trans(vit, EXTR_TRANS_NB, vitref_get_extr_trans(ref, EXTR_TRANS_NB));
  vitfast_set_extr_trans(vit, EXTR_TRANS_EB, vitref_get_extr_trans(ref, EXTR_TRANS_EB));
  vitfast_set_extr_trans(vit, EXTR_TRANS_JB, vitref_get_extr_trans(ref, EXTR_TRANS_JB));
  vitfast_set_extr_trans(vit, EXTR_TRANS_EJ, vitref_get_extr_trans(ref, EXTR_TRANS_EJ));
  vitfast_set_extr_trans(vit, EXTR_TRANS_JJ, vitref_get_extr_trans(ref, EXTR_TRANS_JJ));
  vitfast_set_extr_trans(vit, EXTR_TRANS_EC, vitref_get_extr_trans(ref, EXTR_TRANS_EC));
  vitfast_set_extr_trans(vit, EXTR_TRANS_CC, vitref_get_extr_trans(ref, EXTR_TRANS_CC));
  vitfast_set_extr_trans(vit, EXTR_TRANS_ET, vitref_get_extr_trans(ref, EXTR_TRANS_ET));
  vitfast_set_extr_trans(vit, EXTR_TRANS_CT, vitref_get_extr_trans(ref, EXTR_TRANS_CT));

  for (int i = 0; i < VITREF_TABLE_SIZE; ++i)
  {
    vitfast_set_null(vit, vitref_get_null(ref, i), i);
    vitfast_set_background(vit, vitref_get_background(ref, i), i);
  }

  for (int k = 0; k < K; ++k)
  {
    for (int i = 0; i < VITREF_TABLE_SIZE; ++i)
    vitfast_set_match(vit, vitref_get_match(ref, k, i), k, i);
  }

  for (int k = 0; k < K; ++k)
  {
    vitfast_set_core_trans(vit, CORE_TRANS_BM, vitref_get_core_trans(ref, CORE_TRANS_BM, k), k);
    vitfast_set_core_trans(vit, CORE_TRANS_MM, vitref_get_core_trans(ref, CORE_TRANS_MM, k), k);
    vitfast_set_core_trans(vit, CORE_TRANS_MI, vitref_get_core_trans(ref, CORE_TRANS_MI, k), k);
    vitfast_set_core_trans(vit, CORE_TRANS_MD, vitref_get_core_trans(ref, CORE_TRANS_MD, k), k);
    vitfast_set_core_trans(vit, CORE_TRANS_IM, vitref_get_core_trans(ref, CORE_TRANS_IM, k), k);
    vitfast_set_core_trans(vit, CORE_TRANS_II, vitref_get_core_trans(ref, CORE_TRANS_II, k), k);
    vitfast_set_core_trans(vit, CORE_TRANS_DM, vitref_get_core_trans(ref, CORE_TRANS_DM, k), k);
    vitfast_set_core_trans(vit, CORE_TRANS_DD, vitref_get_core_trans(ref, CORE_TRANS_DD, k), k);
  }
}

static int encode(int size, char const *sequence)
{
  static int emission_limit = 5;
  static int abc_size = 4;
  static int ord[127];
  static int stride[10];
  static char const abc[] = "ACGT";

  for (int i = 0; i < abc_size; ++i)
    ord[(int)abc[i]] = i;

  for (int i = emission_limit - 1; i >= 0; --i)
    stride[i] = ipow(abc_size, i);

  int axis[10];
  for (int i = 0; i < size; ++i)
    axis[i] = ord[(int)sequence[i]];

  return isum(size - 1, stride + 1) + idot(size, axis, stride);
}

static int code_callb(int pos, int len, void *arg)
{
  char const *sequence = arg;
  return encode(len, &sequence[pos]);
}

static void run(struct vitref *ref, struct vitfast *vit, char const *seq)
{
  int L = strlen(seq);
  float slow = vitref_cost(ref, L, code_callb, (void *)seq);
  float fast = vitfast_cost(vit, L, code_callb, (void *)seq);
  close(slow, fast);
}
