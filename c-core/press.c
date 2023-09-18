#include "press.h"
#include "array_size_field.h"
#include "db_writer.h"
#include "defer_return.h"
#include "fs.h"
#include "hmm_reader.h"
#include "p7.h"
#include "rc.h"
#include "sizeof_field.h"
#include "strkcpy.h"
#include <stdlib.h>
#include <string.h>

struct dcp_press
{
  struct
  {
    FILE *fp;
    struct dcp_db_writer db;
  } writer;

  struct
  {
    FILE *fp;
    struct dcp_hmm_reader h3;
  } reader;

  unsigned count;
  struct p7 p7;
  struct imm_nuclt_code code;
  struct dcp_model_params params;
  char buffer[4 * 1024];
};

static int count_proteins(struct dcp_press *);
static int finish_writer(struct dcp_press *);
static int protein_write(struct dcp_press *);

struct dcp_press *dcp_press_new(void)
{
  struct dcp_press *p = malloc(sizeof(struct dcp_press));
  if (!p) return NULL;

  p->writer.fp = NULL;
  p->reader.fp = NULL;
  return p;
}

int dcp_press_setup(struct dcp_press *x, int gencode_id, float epsilon)
{
  x->params.gencode = imm_gencode_get((unsigned)gencode_id);
  if (!x->params.gencode) return DCP_EGENCODEID;
  x->params.amino = &imm_amino_iupac;
  imm_nuclt_code_init(&x->code, &imm_dna_iupac.super);
  x->params.code = &x->code;
  x->params.entry_dist = DCP_ENTRY_DIST_OCCUPANCY;
  x->params.epsilon = epsilon;
  return 0;
}

int dcp_press_open(struct dcp_press *x, char const *hmm, char const *db)
{
  x->writer.fp = NULL;
  x->reader.fp = NULL;

  int rc = 0;

  if (!(x->reader.fp = fopen(hmm, "rb"))) defer_return(DCP_EOPENHMM);
  if (!(x->writer.fp = fopen(db, "wb"))) defer_return(DCP_EOPENDB);

  if ((rc = count_proteins(x))) defer_return(rc);

  dcp_db_writer_init(&x->writer.db, x->params);
  if ((rc = dcp_db_writer_open(&x->writer.db, x->writer.fp))) defer_return(rc);

  dcp_hmm_reader_init(&x->reader.h3, x->params, x->reader.fp);

  p7_init(&x->p7, x->params);

  char const *acc = x->reader.h3.protein.meta.acc;
  if ((rc = p7_set_accession(&x->p7, acc))) defer_return(rc);

  return rc;

defer:
  if (x->writer.fp) fclose(x->writer.fp);
  if (x->reader.fp) fclose(x->reader.fp);
  x->writer.fp = NULL;
  x->reader.fp = NULL;
  return rc;
}

long dcp_press_nproteins(struct dcp_press const *press) { return press->count; }

static int count_proteins(struct dcp_press *press)
{
#define HMMER3 "HMMER3/f"

  unsigned count = 0;
  int bufsize = sizeof_field(struct dcp_press, buffer);
  while (fgets(press->buffer, bufsize, press->reader.fp) != NULL)
  {
    if (!strncmp(press->buffer, HMMER3, strlen(HMMER3))) ++count;
  }

  if (!feof(press->reader.fp)) return DCP_EFREAD;

  press->count = count;
  rewind(press->reader.fp);
  return 0;

#undef HMMER3
}

int dcp_press_next(struct dcp_press *press)
{
  int rc = dcp_hmm_reader_next(&press->reader.h3);
  if (rc) return rc;

  if (dcp_hmm_reader_end(&press->reader.h3)) return 0;

  return protein_write(press);
}

bool dcp_press_end(struct dcp_press const *press)
{
  return dcp_hmm_reader_end(&press->reader.h3);
}

int dcp_press_close(struct dcp_press *press)
{
  int rc_r = press->reader.fp ? dcp_fs_close(press->reader.fp) : 0;
  int rc_w = finish_writer(press);
  press->writer.fp = NULL;
  press->reader.fp = NULL;
  p7_cleanup(&press->p7);
  dcp_hmm_reader_cleanup(&press->reader.h3);
  return rc_r ? rc_r : (rc_w ? rc_w : 0);
}

void dcp_press_del(struct dcp_press const *x) { free((void *)x); }

static int finish_writer(struct dcp_press *press)
{
  if (!press->writer.fp) return 0;

  int rc = dcp_db_writer_close(&press->writer.db);
  if (rc) defer_return(rc);

  return dcp_fs_close(press->writer.fp);

defer:
  fclose(press->writer.fp);
  return rc;
}

static int protein_write(struct dcp_press *x)
{
  int rc = p7_absorb(&x->p7, &x->reader.h3.model);
  if (rc) return rc;

  size_t n = array_size_field(struct p7, accession);
  if (!strkcpy(x->p7.accession, x->reader.h3.protein.meta.acc, n))
    return DCP_EFORMAT;

  return dcp_db_writer_pack(&x->writer.db, &x->p7);
}
