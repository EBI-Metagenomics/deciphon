#include "product_thread.h"
#include "array_size_field.h"
#include "defer_return.h"
#include "format.h"
#include "fs.h"
#include "hmmer_result.h"
#include "imm/imm.h"
#include "match.h"
#include "match_iter.h"
#include "rc.h"
#include <stdarg.h>
#include <string.h>

int product_thread_init(struct product_thread *x, int tid, char const *dir)
{
  int rc = 0;
  x->id = tid;
  x->dirname = dir;
  char *name = x->filename;
  size_t n = array_size_field(struct product_thread, filename);
  if ((rc = format(name, n, "%s/.products.%03d.tsv", dir, tid))) return rc;
  if ((rc = fs_touch(x->filename))) return rc;
  product_line_init(&x->line);
  return 0;
}

static int write_match(FILE *, struct match const *);

int product_thread_put_match(struct product_thread *x, struct match *match,
                             struct match_iter *it)
{
  int rc = 0;
  struct product_line *line = &x->line;

  FILE *fp = fopen(x->filename, "ab");
  if (!fp) return DCP_EFOPEN;

  if (fprintf(fp, "%ld\t", line->sequence) < 0) defer_return(DCP_EWRITEPROD);
  if (fprintf(fp, "%d\t", line->window) < 0) defer_return(DCP_EWRITEPROD);
  if (fprintf(fp, "%d\t", line->window_start) < 0) defer_return(DCP_EWRITEPROD);
  if (fprintf(fp, "%d\t", line->window_stop) < 0) defer_return(DCP_EWRITEPROD);
  if (fprintf(fp, "%s\t", line->protein) < 0) defer_return(DCP_EWRITEPROD);
  if (fprintf(fp, "%s\t", line->abc) < 0) defer_return(DCP_EWRITEPROD);
  if (fprintf(fp, "%6.1f\t", line->lrt) < 0) defer_return(DCP_EWRITEPROD);
  if (fprintf(fp, "%9.2g\t", line->evalue) < 0) defer_return(DCP_EWRITEPROD);

  int i = 0;
  while (!(rc = match_iter_next(it, match)) && !match_iter_end(it))
  {
    if (i++ && fputc(';', fp) == EOF) defer_return(DCP_EWRITEPROD);
    if ((rc = write_match(fp, match))) defer_return(rc);
  }

  if (fputc('\n', fp) == EOF) defer_return(DCP_EWRITEPROD);

  return fclose(fp) ? DCP_EFCLOSE : 0;

defer:
  fclose(fp);
  return rc;
}

int product_thread_put_hmmer(struct product_thread *x,
                             struct hmmer_result const *result)
{
  char file[DCP_PATH_MAX] = {0};
  int rc = 0;
  char const *dir = x->dirname;
  long seq = x->line.sequence;
  char *prot = x->line.protein;

  if ((rc = format(file, DCP_PATH_MAX, "%s/hmmer/%ld", dir, seq))) return rc;
  if ((rc = fs_mkdir(file, true))) return rc;
  if ((rc = format(file, DCP_PATH_MAX, "%s/hmmer/%ld/%s.h3r", dir, seq, prot)))
    return rc;

  FILE *fp = fopen(file, "wb");
  if (!fp) return DCP_EFOPEN;

  if ((rc = hmmer_result_pack(result, fp)))
  {
    fclose(fp);
    return rc;
  }

  return fclose(fp) ? DCP_EFCLOSE : 0;
}

static int write_match(FILE *fp, struct match const *m)
{
  char buff[IMM_STATE_NAME_SIZE + 20] = {0};

  char *ptr = buff;
  memcpy(ptr, imm_seq_data(&m->seq), (unsigned)imm_seq_size(&m->seq));
  ptr += imm_seq_size(&m->seq);
  *ptr++ = ',';

  match_state_name(m, ptr);
  ptr += strlen(ptr);
  *ptr++ = ',';

  if (!match_state_is_mute(m))
  {
    struct imm_codon codon = match_codon(m);
    *ptr++ = imm_codon_asym(&codon);
    *ptr++ = imm_codon_bsym(&codon);
    *ptr++ = imm_codon_csym(&codon);
  }

  *ptr++ = ',';

  if (!match_state_is_mute(m)) *ptr++ = match_amino(m);

  *ptr = '\0';

  return fputs(buff, fp) == EOF ? DCP_EFWRITE : 0;
}
