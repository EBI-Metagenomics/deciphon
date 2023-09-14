#include "prod_writer_thrd.h"
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

/* Reference: https://stackoverflow.com/a/21162120 */
#define DBL_FMT "%.17g"

#define fmt(B, N, F, ...) dcp_format((B), (N), (F), __VA_ARGS__)
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

int dcp_prod_writer_thrd_init(struct dcp_prod_writer_thrd *x, int idx,
                              char const *dir)
{
  int rc = 0;
  x->idx = idx;
  x->dirname = dir;
  size_t n = array_size_field(struct dcp_prod_writer_thrd, prodname);
  if ((rc = fmt(x->prodname, n, "%s/.products.%03d.tsv", dir, idx))) return rc;
  if ((rc = dcp_fs_touch(x->prodname))) return rc;
  dcp_prod_match_init(&x->match);
  return 0;
}

static int write_begin(FILE *, struct dcp_prod_match const *);
static int write_match(FILE *, struct dcp_match const *);
static int write_sep(FILE *);
static int write_end(FILE *);

int dcp_prod_writer_thrd_put(struct dcp_prod_writer_thrd *x,
                             struct dcp_match *match, struct dcp_match_iter *it)
{
  int rc = 0;

  FILE *fp = fopen(x->prodname, "ab");
  if (!fp) return DCP_EFOPEN;

  if ((rc = write_begin(fp, &x->match))) defer_return(rc);

  int i = 0;
  while (!(rc = dcp_match_iter_next(it, match)))
  {
    if (dcp_match_iter_end(it)) break;
    if (i++ && (rc = write_sep(fp))) defer_return(rc);
    if ((rc = write_match(fp, match))) defer_return(rc);
  }

  if ((rc = write_end(fp))) defer_return(rc);

  return fclose(fp) ? DCP_EFCLOSE : 0;

defer:
  fclose(fp);
  return rc;
}

int dcp_prod_writer_thrd_put_hmmer(struct dcp_prod_writer_thrd *x,
                                   struct dcp_hmmer_result const *result)
{
  char file[DCP_PATH_MAX] = {0};
  int rc = 0;
  char const *dirname = x->dirname;

  if ((rc = FMT(file, "%s/hmmer/%ld", dirname, x->match.seq_id))) return rc;
  if ((rc = dcp_fs_mkdir(file, true))) return rc;

  if ((rc = FMT(file, "%s/hmmer/%ld/%s.h3r", dirname, x->match.seq_id,
                x->match.protein)))
    return rc;

  FILE *fp = fopen(file, "wb");
  if (!fp) return DCP_EFOPEN;

  if ((rc = dcp_hmmer_result_pack(result, fp)))
  {
    fclose(fp);
    return rc;
  }

  return fclose(fp) ? DCP_EFCLOSE : 0;
}

static int write_begin(FILE *fp, struct dcp_prod_match const *y)
{
  if (fprintf(fp, "%ld\t", y->seq_id) < 0) return DCP_EWRITEPROD;

  if (fprintf(fp, "%s\t", y->protein) < 0) return DCP_EWRITEPROD;
  if (fprintf(fp, "%s\t", y->abc) < 0) return DCP_EWRITEPROD;

  char const *f32 = imm_fmt_get_f32();

  if (fprintf(fp, f32, y->alt) < 0) return DCP_EWRITEPROD;
  if (fputc('\t', fp) < 0) return DCP_EWRITEPROD;

  if (fprintf(fp, f32, y->null) < 0) return DCP_EWRITEPROD;
  if (fputc('\t', fp) < 0) return DCP_EWRITEPROD;

  if (fprintf(fp, f32, y->evalue) < 0) return DCP_EWRITEPROD;
  if (fputc('\t', fp) < 0) return DCP_EWRITEPROD;

  return 0;
}

static int write_match(FILE *fp, struct dcp_match const *m)
{
  char buff[IMM_STATE_NAME_SIZE + 20] = {0};

  char *ptr = buff;
  memcpy(ptr, m->seq.str, m->seq.size);
  ptr += m->seq.size;
  *ptr++ = ',';

  dcp_match_state_name(m, ptr);
  ptr += strlen(ptr);
  *ptr++ = ',';

  if (!dcp_match_state_is_mute(m))
  {
    struct imm_codon codon = dcp_match_codon(m);
    *ptr++ = imm_codon_asym(&codon);
    *ptr++ = imm_codon_bsym(&codon);
    *ptr++ = imm_codon_csym(&codon);
  }

  *ptr++ = ',';

  if (!dcp_match_state_is_mute(m)) *ptr++ = dcp_match_amino(m);

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
