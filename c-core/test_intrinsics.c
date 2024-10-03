#include "aye.h"
#include "intrinsics.h"

static void test_shift(void)
{
  packf x = dupf(1);
  for (int i = 0; i < NUM_LANES; ++i)
  {
    x = shift(x);
    for (int j = 0; j < i; ++j)
      aye(isinf(getf(x, j)));
  }
}

static void test_hmaxu(void)
{
  packu x = dupu(0);
  for (int i = 0; i < NUM_LANES; ++i)
  {
    setu(&x, i + 1, i);
    aye(hmaxu(x) == (u32)(i + 1));
  }
}

static void test_hmin(void)
{
  packf x = dupf(INFINITY);
  for (int i = 0; i < NUM_LANES; ++i)
  {
    setf(&x, NUM_LANES - i, i);
    aye(hmin(x) == (f32)(NUM_LANES - i));
  }
}

static void test_hmin_idx(void)
{
  for (int i = 0; i < NUM_LANES; ++i)
  {
    packf a = dupf(INFINITY);
    packf b = dupf(INFINITY);
    packu ai = dupu(0);
    packu bi = dupu(0);

    setf(&b, 0, i);
    setu(&bi, 1, i);

    min_idx(&a, b, &ai, bi);
    aye(getf(a, i) == (f32)(0));
    aye(getu(ai, i) == (u32)(1));
  }
}

static void test_all_leq(void)
{
  packf a = dupf(INFINITY);
  packf b = dupf(INFINITY);
  aye(all_leq(a, b));
  for (int i = 0; i < NUM_LANES; ++i)
  {
    a = dupf(INFINITY);
    b = dupf(INFINITY);
    setf(&a, 0, i);
    aye(all_leq(a, b));
  }
  for (int i = 0; i < NUM_LANES; ++i)
  {
    a = dupf(INFINITY);
    b = dupf(INFINITY);
    setf(&b, 0, i);
    aye(!all_leq(a, b));
  }
}

int main(void)
{
  aye_begin();
  test_shift();
  test_hmaxu();
  test_hmin();
  test_hmin_idx();
  test_all_leq();
  return aye_end();
}
