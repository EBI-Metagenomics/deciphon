#include "protein_reader.h"
#include "database_reader.h"
#include "defer_return.h"
#include "error.h"
#include "expect.h"
#include "fs.h"
#include "min.h"
#include "partition_size.h"
#include "protein_iter.h"
#include "read.h"
#include "sizeof_field.h"
#include <fcntl.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>

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
  if (num_partitions > PROTEIN_READER_MAX_PARTITIONS)
    return error(DCP_EMANYPARTS);
  x->num_partitions = min(num_partitions, db->num_proteins);

  if ((rc = expect_key(&db->file, "proteins"))) return rc;

  uint32_t num_proteins = 0;
  if ((rc = read_array(&db->file, &num_proteins))) return rc;
  if (num_proteins > INT_MAX) return error(DCP_EFDATA);
  if ((int)num_proteins != db->num_proteins) return error(DCP_EFDATA);

  if (lio_tell(&db->file, &x->offset[0])) return error(DCP_EFTELL);
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

  int fd = lio_rfile(&x->db->file);
  int newfd = -1;
  int rc = 0;
  long offset = x->offset[partition];

  if ((rc = fs_reopen(fd, O_RDONLY, &newfd))) defer_return(rc);
  if ((rc = fs_seek(newfd, offset, SEEK_SET))) defer_return(rc);

  int start = x->size_cumsum[partition];
  int end = start + protein_reader_partition_size(x, partition);
  protein_iter_init(it, x, start, end, offset, newfd);

  return rc;

defer:
  if (newfd >= 0) close(newfd);
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
