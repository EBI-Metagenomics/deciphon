#include "press.h"
#include "array_size_field.h"
#include "database_writer.h"
#include "defer_return.h"
#include "fs.h"
#include "hmm_reader.h"
#include "protein.h"
#include "rc.h"
#include "sizeof_field.h"
#include "xstrcpy.h"
#include <stdlib.h>
#include <string.h>

struct press
{
  struct
  {
    FILE *fp;
    struct database_writer db;
  } writer;

  struct
  {
    FILE *fp;
    struct hmm_reader h3;
  } reader;

  int count;
  struct protein protein;
  struct imm_nuclt_code code;
  struct model_params params;
  char buffer[4 * 1024];
};

static int count_proteins(struct press *);
static int finish_writer(struct press *);
static int protein_write(struct press *);

struct press *press_new(void)
{
  struct press *x = malloc(sizeof(*x));
  if (!x) return NULL;

  x->writer.fp = NULL;
  x->reader.fp = NULL;
  protein_init(&x->protein);
  return x;
}

int press_setup(struct press *x, int gencode_id, float epsilon)
{
  x->params.gencode = imm_gencode_get(gencode_id);
  if (!x->params.gencode) return DCP_EGENCODEID;
  x->params.amino = &imm_amino_iupac;
  imm_nuclt_code_init(&x->code, &imm_dna_iupac.super);
  x->params.code = &x->code;
  x->params.entry_dist = ENTRY_DIST_OCCUPANCY;
  x->params.epsilon = epsilon;
  return 0;
}

int press_open(struct press *x, char const *hmm, char const *db)
{
  x->writer.fp = NULL;
  x->reader.fp = NULL;

  int rc = 0;

  if (!(x->reader.fp = fopen(hmm, "rb"))) defer_return(DCP_EOPENHMM);
  if (!(x->writer.fp = fopen(db, "wb"))) defer_return(DCP_EOPENDB);

  if ((rc = count_proteins(x))) defer_return(rc);

  database_writer_init(&x->writer.db, x->params);
  if ((rc = database_writer_open(&x->writer.db, x->writer.fp)))
    defer_return(rc);

  if ((rc = hmm_reader_init(&x->reader.h3, x->params, x->reader.fp)))
    defer_return(rc);

  protein_setup(&x->protein, x->params);

  char const *acc = x->reader.h3.protein.meta.acc;
  if ((rc = protein_set_accession(&x->protein, acc)))
  {
    hmm_reader_cleanup(&x->reader.h3);
    defer_return(rc);
  }

  return rc;

defer:
  if (x->writer.fp) fclose(x->writer.fp);
  if (x->reader.fp) fclose(x->reader.fp);
  x->writer.fp = NULL;
  x->reader.fp = NULL;
  return rc;
}

long press_nproteins(struct press const *press) { return press->count; }

static int count_proteins(struct press *press)
{
#define HMMER3 "HMMER3/f"

  int count = 0;
  int bufsize = sizeof_field(struct press, buffer);
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

int press_next(struct press *press)
{
  int rc = hmm_reader_next(&press->reader.h3);
  if (rc) return rc;

  if (hmm_reader_end(&press->reader.h3)) return 0;

  return protein_write(press);
}

bool press_end(struct press const *press)
{
  return hmm_reader_end(&press->reader.h3);
}

int press_close(struct press *press)
{
  int rc_r = press->reader.fp ? fs_close(press->reader.fp) : 0;
  int rc_w = finish_writer(press);
  press->writer.fp = NULL;
  press->reader.fp = NULL;
  protein_cleanup(&press->protein);
  hmm_reader_cleanup(&press->reader.h3);
  return rc_r ? rc_r : (rc_w ? rc_w : 0);
}

void press_del(struct press const *x)
{
  if (x)
  {
    protein_cleanup((struct protein *)&x->protein);
    free((void *)x);
  }
}

static int finish_writer(struct press *press)
{
  if (!press->writer.fp) return 0;

  int rc = database_writer_close(&press->writer.db);
  if (rc) defer_return(rc);

  return fs_close(press->writer.fp);

defer:
  fclose(press->writer.fp);
  return rc;
}

static int protein_write(struct press *x)
{
  int rc = protein_absorb(&x->protein, &x->reader.h3.model);
  if (rc) return rc;

  size_t n = array_size_field(struct protein, accession);
  if (!xstrcpy(x->protein.accession, x->reader.h3.protein.meta.acc, n))
    return DCP_ELONGACCESSION;

  return database_writer_pack(&x->writer.db, &x->protein);
}
