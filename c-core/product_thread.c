#include "product_thread.h"
#include "array_size.h"
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

#define fmt(B, N, F, ...) format((B), (N), (F), __VA_ARGS__)
#define FMT(buf, format, ...) fmt((buf), array_size(buf), (format), __VA_ARGS__)

/* Output example for two matches.
 *
 *             ___________________________
 *             |   match0   |   match1   |
 *             ---------------------------
 * Output----->| CG,M1,CGA,K;CG,M4,CGA,K |
 *             ---|-|---|--|--------------
 * -----------   /  |   |  \    ---------------
 * | matched |__/   |   |   \___| most likely |
 * | letters |      |   |       | amino acid  |
 * -----------      |   |       ---------------
 *      -------------   ---------------
 *      | hmm state |   | most likely |
 *      -------------   | codon       |
 *                      ---------------
 */

int product_thread_init(struct product_thread *x, int idx, char const *dir)
{
  int rc = 0;
  x->idx = idx;
  x->dirname = dir;
  size_t n = array_size_field(struct product_thread, prodname);
  if ((rc = fmt(x->prodname, n, "%s/.products.%03d.tsv", dir, idx))) return rc;
  if ((rc = fs_touch(x->prodname))) return rc;
  product_line_init(&x->line);
  return 0;
}

static int write_begin(FILE *, struct product_line const *);
static int write_match(FILE *, struct match const *);
static int write_sep(FILE *);
static int write_end(FILE *);

int product_thread_put(struct product_thread *x, struct match *match,
                       struct match_iter *it)
{
  int rc = 0;

  FILE *fp = fopen(x->prodname, "ab");
  if (!fp) return DCP_EFOPEN;

  if ((rc = write_begin(fp, &x->line))) defer_return(rc);

  int i = 0;
  while (!(rc = match_iter_next(it, match)))
  {
    if (match_iter_end(it)) break;
    if (i++ && (rc = write_sep(fp))) defer_return(rc);
    if ((rc = write_match(fp, match))) defer_return(rc);
  }

  if ((rc = write_end(fp))) defer_return(rc);

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
  char const *dirname = x->dirname;

  if ((rc = FMT(file, "%s/hmmer/%ld", dirname, x->line.sequence))) return rc;
  if ((rc = fs_mkdir(file, true))) return rc;

  if ((rc = FMT(file, "%s/hmmer/%ld/%s.h3r", dirname, x->line.sequence,
                x->line.protein)))
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

static int write_begin(FILE *fp, struct product_line const *y)
{
  if (fprintf(fp, "%ld\t", y->sequence) < 0) return DCP_EWRITEPROD;

  if (fprintf(fp, "%d\t", y->window) < 0) return DCP_EWRITEPROD;
  if (fprintf(fp, "%d\t", y->window_start) < 0) return DCP_EWRITEPROD;
  if (fprintf(fp, "%d\t", y->window_stop) < 0) return DCP_EWRITEPROD;

  if (fprintf(fp, "%s\t", y->protein) < 0) return DCP_EWRITEPROD;
  if (fprintf(fp, "%s\t", y->abc) < 0) return DCP_EWRITEPROD;

  if (fprintf(fp, "%6.1f\t", y->lrt) < 0) return DCP_EWRITEPROD;
  if (fprintf(fp, "%9.2g\t", y->evalue) < 0) return DCP_EWRITEPROD;
  return 0;
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

static int write_sep(FILE *fp)
{
  return fputc(';', fp) == EOF ? DCP_EWRITEPROD : 0;
}

static int write_end(FILE *fp)
{
  return fputc('\n', fp) == EOF ? DCP_EWRITEPROD : 0;
}
