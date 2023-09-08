#include "protein_iter.h"
#include "db_reader.h"
#include "fs.h"
#include "p7.h"
#include "protein.h"
#include "protein_reader.h"

void dcp_protein_iter_init(struct dcp_protein_iter *x,
                           struct dcp_protein_reader *reader, int partition,
                           int start_idx, long offset, FILE *fp)
{
  x->partition = partition;
  x->start_idx = start_idx;
  x->curr_idx = start_idx - 1;
  x->offset = offset;
  x->fp = fp;
  lip_file_init(&x->file, fp);
  x->reader = reader;
}

int dcp_protein_iter_rewind(struct dcp_protein_iter *x)
{
  x->curr_idx = x->start_idx - 1;
  return dcp_fs_seek(x->fp, x->offset, SEEK_SET);
}

int dcp_protein_iter_next(struct dcp_protein_iter *x,
                          struct dcp_protein *protein, struct p7 *p7)
{
  x->curr_idx += 1;
  if (dcp_protein_iter_end(x)) return 0;

  // int rc = dcp_protein_unpack(protein, &x->file);
  // if (rc) return rc;

  int rc = p7_unpack(p7, &x->file);
  if (rc) return rc;

  long offset = 0;
  dcp_fs_tell(x->fp, &offset);
  return rc;
}

bool dcp_protein_iter_end(struct dcp_protein_iter const *x)
{
  int size = dcp_protein_reader_partition_size(x->reader, x->partition);
  return x->start_idx + size == x->curr_idx;
}

int dcp_protein_iter_idx(struct dcp_protein_iter const *x)
{
  return x->curr_idx;
}
