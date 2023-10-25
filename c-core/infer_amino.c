#include "infer_amino.h"
#include "chararray.h"
#include "match.h"
#include "match_iter.h"

int infer_amino(struct chararray *x, struct match *match, struct match_iter *it)
{
  int rc = 0;

  chararray_reset(x);
  while (!(rc = match_iter_next(it, match)))
  {
    if (match_iter_end(it)) break;
    if (!match_state_is_core(match)) continue;
    if (match_state_is_mute(match)) continue;
    if ((rc = chararray_append(x, match_amino(match)))) return rc;
  }

  return chararray_append(x, '\0');
}
