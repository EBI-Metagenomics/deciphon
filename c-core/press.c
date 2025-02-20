#include "array_size_field.h"
#include "database_writer.h"
#include "deciphon.h"
#include "defer_return.h"
#include "error.h"
#include "fs.h"
#include "hmm_reader.h"
#include "imm_gencode.h"
#include "protein.h"
#include "sizeof_field.h"
#include "xstrcpy.h"
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct dcp_press
{
  struct
  {
    int fd;
    struct database_writer db;
  } writer;

  struct
  {
    FILE *fp;
    struct hmm_reader h3;
  } reader;

  int count;
  bool has_ga;
  struct protein protein;
  struct imm_nuclt_code code;
  struct model_params params;
  char buffer[4 * 1024];
};

static int count_proteins(struct dcp_press *);
static int finish_writer(struct dcp_press *);
static int protein_write(struct dcp_press *);

struct dcp_press *dcp_press_new(void)
{
  struct dcp_press *x = malloc(sizeof(*x));
  if (!x) return NULL;

  x->writer.fd = -1;
  x->reader.fp = NULL;
  x->has_ga = true;
  protein_init(&x->protein);
  return x;
}

int dcp_press_setup(struct dcp_press *x, int gencode_id, float epsilon)
{
  x->params.gencode = imm_gencode_get(gencode_id);
  if (!x->params.gencode) return error(DCP_EGENCODEID);
  x->params.amino = &imm_amino_iupac;
  imm_nuclt_code_init(&x->code, &imm_dna_iupac.super);
  x->params.code = &x->code;
  x->params.entry_dist = ENTRY_DIST_OCCUPANCY;
  x->params.epsilon = epsilon;
  return 0;
}

int dcp_press_open(struct dcp_press *x, char const *hmm, char const *db)
{
  x->writer.fd = -1;
  x->reader.fp = NULL;

  int rc = 0;

  if ((rc = fs_fopen(&x->reader.fp, hmm, "rb")))                             defer_return(error(rc));
  if ((rc = fs_open(&x->writer.fd, db, O_WRONLY | O_CREAT | O_TRUNC, 0644))) defer_return(error(rc));

  if ((rc = count_proteins(x))) defer_return(rc);

  database_writer_init(&x->writer.db, x->params);
  if ((rc = database_writer_open(&x->writer.db, x->writer.fd)))
    defer_return(rc);

  if ((rc = hmm_reader_init(&x->reader.h3, x->params, x->reader.fp)))
    defer_return(rc);

  protein_setup(&x->protein, x->params, true, false);

  char const *acc = x->reader.h3.protein.meta.acc;
  if ((rc = protein_set_accession(&x->protein, acc)))
  {
    hmm_reader_cleanup(&x->reader.h3);
    defer_return(rc);
  }

  return rc;

defer:
  if (x->writer.fd) close(x->writer.fd);
  if (x->reader.fp) fs_fclose(x->reader.fp);
  x->writer.fd = -1;
  x->reader.fp = NULL;
  return rc;
}

long dcp_press_nproteins(struct dcp_press const *press) { return press->count; }

static int count_proteins(struct dcp_press *press)
{
#define HMMER3 "HMMER3/f"

  int count = 0;
  int bufsize = sizeof_field(struct dcp_press, buffer);
  while (fgets(press->buffer, bufsize, press->reader.fp) != NULL)
  {
    if (!strncmp(press->buffer, HMMER3, strlen(HMMER3))) ++count;
  }

  if (!feof(press->reader.fp)) return error(DCP_EFREAD);

  press->count = count;
  rewind(press->reader.fp);
  return 0;

#undef HMMER3
}

int dcp_press_next(struct dcp_press *press)
{
  int rc = hmm_reader_next(&press->reader.h3);
  if (rc) return rc;

  if (hmm_reader_end(&press->reader.h3)) return 0;

  return protein_write(press);
}

bool dcp_press_end(struct dcp_press const *press)
{
  return hmm_reader_end(&press->reader.h3);
}

int dcp_press_close(struct dcp_press *press)
{
  int rc_r = error(press->reader.fp ? fs_fclose(press->reader.fp) : 0);
  int rc_w = error(finish_writer(press));
  press->writer.fd = -1;
  press->reader.fp = NULL;
  protein_cleanup(&press->protein);
  hmm_reader_cleanup(&press->reader.h3);
  if (rc_r) return rc_r;
  if (rc_w) return rc_w;
  return 0;
}

void dcp_press_del(struct dcp_press const *x)
{
  if (x)
  {
    protein_cleanup((struct protein *)&x->protein);
    free((void *)x);
  }
}

static int finish_writer(struct dcp_press *press)
{
  if (!press->writer.fd) return 0;

  database_writer_set_has_ga(&press->writer.db, press->has_ga);
  int rc = database_writer_close(&press->writer.db);
  if (rc) defer_return(rc);

  return error(fs_close(press->writer.fd));

defer:
  fs_close(press->writer.fd);
  return error(rc);
}

static int protein_write(struct dcp_press *x)
{
  int rc = protein_absorb(&x->protein, &x->reader.h3.model);
  if (rc) return rc;

  if (!x->protein.has_ga) x->has_ga = false;
  size_t n = array_size_field(struct protein, accession);
  if (xstrcpy(x->protein.accession, x->reader.h3.protein.meta.acc, n))
    return error(DCP_ELONGACCESSION);

  return database_writer_pack(&x->writer.db, &x->protein);
}
