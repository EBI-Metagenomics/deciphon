#include "protein_reader.h"
#include "array_size_field.h"
#include "database_reader.h"
#include "defer_return.h"
#include "error.h"
#include "fs.h"
#include "imm/imm.h"
#include "partition_size.h"
#include "protein_iter.h"
#include "rc.h"
#include "unpack.h"
#include <string.h>

void protein_reader_init(struct protein_reader *x)
{
  x->num_partitions = 0;
  memset(x->size_cumsum, 0, sizeof_field(struct protein_reader, size_cumsum));
  memset(x->offset, 0, sizeof_field(struct protein_reader, offset));
  x->db = NULL;
}

static void partition_it(struct protein_reader *);

int protein_reader_setup(struct protein_reader *x, struct database_reader *db,
                         int num_partitions)
{
  int rc = 0;
  x->db = db;

  if (num_partitions == 0) return error(DCP_EZEROPART);
  if (num_partitions > DCP_NPARTITIONS_MAX) return error(DCP_EMANYPARTS);
  x->num_partitions = imm_min(num_partitions, db->num_proteins);

  if ((rc = unpack_key(&db->file, "proteins"))) return rc;

  unsigned num_proteins = 0;
  if (!lip_read_array_size(&db->file, &num_proteins)) return error(DCP_EFREAD);
  if (num_proteins > INT_MAX) return error(DCP_EFDATA);
  if ((int)num_proteins != db->num_proteins) return error(DCP_EFDATA);

  if ((rc = fs_tell(db->file.fp, &x->offset[0]))) return rc;
  partition_it(x);

  return rc;
}

int protein_reader_num_partitions(struct protein_reader const *x)
{
  return x->num_partitions;
}

int protein_reader_partition_size(struct protein_reader const *x, int partition)
{
  int const *csum = x->size_cumsum;
  return csum[partition + 1] - csum[partition];
}

int protein_reader_size(struct protein_reader const *x)
{
  return x->size_cumsum[x->num_partitions];
}

int protein_reader_iter(struct protein_reader *x, int partition,
                        struct protein_iter *it)
{
  if (partition < 0 || x->num_partitions < partition)
    return error(DCP_EINVALPART);

  FILE *fp = lip_file_ptr(&x->db->file);
  FILE *newfp = NULL;
  int rc = 0;
  long offset = x->offset[partition];

  if ((rc = fs_refopen(fp, "rb", &newfp))) defer_return(rc);
  if ((rc = fs_seek(newfp, offset, SEEK_SET))) defer_return(rc);

  int start = x->size_cumsum[partition];
  int end = start + protein_reader_partition_size(x, partition);
  protein_iter_init(it, x, start, end, offset, newfp);

  return rc;

defer:
  if (newfp) fclose(newfp);
  return rc;
}

static void partition_it(struct protein_reader *x)
{
  int n = x->db->num_proteins;
  int k = x->num_partitions;
  int part = 0;
  for (int i = 0; i < k; ++i)
  {
    int size = (int)partition_size(n, k, i);

    x->size_cumsum[i + 1] = x->size_cumsum[i] + size;

    for (int j = 0; j < size; ++j)
      x->offset[i + 1] += x->db->protein_sizes[part++];

    x->offset[i + 1] += x->offset[i];
  }
}
