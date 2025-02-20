#include "product_thread.h"
#include "array_size_field.h"
#include "deciphon.h"
#include "defer_return.h"
#include "error.h"
#include "format.h"
#include "fs.h"
#include "h3r_result.h"
#include "imm_codon.h"
#include "match.h"
#include <fcntl.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

void product_thread_init(struct product_thread *x, int id)
{
  x->id = id;
  x->dirname = NULL;
  memset(x->filename, 0, FS_PATH_MAX);
  product_line_init(&x->line);
}

int product_thread_setup(struct product_thread *x, char const *abc, char const *dir)
{
  int rc = 0;
  x->dirname = dir;
  char *name = x->filename;
  size_t n = array_size_field(struct product_thread, filename);
  if ((rc = format(name, n, "%s/.products.%03d.tsv", dir, x->id))) return rc;
  if ((rc = fs_touch(x->filename))) return rc;
  product_line_init(&x->line);
  product_line_set_abc(&x->line, abc);
  return 0;
}

static int write_match(FILE *, struct match const *);

int product_thread_add_match(struct product_thread *x, struct match begin,
                             struct match end)
{
#define defer_error(x) defer_return(error(x))
  int rc = 0;
  struct product_line *line = &x->line;

  FILE *fp = NULL;
  if ((rc = fs_fopen(&fp, x->filename, "ab"))) return error(rc);

  if (fprintf(fp, "%ld\t",  line->sequence)       < 0) defer_error(DCP_EWRITEPROD);
  if (fprintf(fp, "%d\t",   line->window)         < 0) defer_error(DCP_EWRITEPROD);
  if (fprintf(fp, "%d\t",   line->window_start)   < 0) defer_error(DCP_EWRITEPROD);
  if (fprintf(fp, "%d\t",   line->window_stop)    < 0) defer_error(DCP_EWRITEPROD);
  if (fprintf(fp, "%d\t",   line->hit)            < 0) defer_error(DCP_EWRITEPROD);
  if (fprintf(fp, "%d\t",   line->hit_start)      < 0) defer_error(DCP_EWRITEPROD);
  if (fprintf(fp, "%d\t",   line->hit_stop)       < 0) defer_error(DCP_EWRITEPROD);
  if (fprintf(fp, "%s\t",   line->protein)        < 0) defer_error(DCP_EWRITEPROD);
  if (fprintf(fp, "%s\t",   line->abc)            < 0) defer_error(DCP_EWRITEPROD);
  if (fprintf(fp, "%.1f\t", line->lrt)            < 0) defer_error(DCP_EWRITEPROD);
  if (fprintf(fp, "%.2g\t", exp(line->logevalue)) < 0) defer_error(DCP_EWRITEPROD);

  int i = 0;
  struct match it = begin;
  while (!match_equal(it, end))
  {
    if (i++ && fputc(';', fp) == EOF) defer_error(DCP_EWRITEPROD);
    if ((rc = write_match(fp, &it)))  defer_error(rc);
    it = match_next(&it);
  }

  if (fputc('\n', fp) == EOF) defer_error(DCP_EWRITEPROD);

  return error(fs_fclose(fp));

defer:
  fs_fclose(fp);
  return rc;
#undef defer_error
}

int product_thread_add_hmmer(struct product_thread *x, struct h3r const *result)
{
  char file[FS_PATH_MAX] = {0};
  int rc = 0;
  char const *dir = x->dirname;
  long seq = x->line.sequence;
  int win = x->line.window;
  int hit = x->line.hit;
  char *prot = x->line.protein;

  if ((rc = format(file, FS_PATH_MAX, "%s/hmmer/%ld", dir, seq)))                              return rc;
  if ((rc = fs_mkdir(file, true)))                                                             return rc;
  if ((rc = format(file, FS_PATH_MAX, "%s/hmmer/%ld/%d", dir, seq, win)))                      return rc;
  if ((rc = fs_mkdir(file, true)))                                                             return rc;
  if ((rc = format(file, FS_PATH_MAX, "%s/hmmer/%ld/%d/%d", dir, seq, win, hit)))              return rc;
  if ((rc = fs_mkdir(file, true)))                                                             return rc;
  if ((rc = format(file, FS_PATH_MAX, "%s/hmmer/%ld/%d/%d/%s.h3r", dir, seq, win, hit, prot))) return rc;

  int fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (fd < 0) return error(DCP_EOPEN);

  if ((rc = h3r_pack(result, fd)))
  {
    fs_close(fd);
    return rc;
  }

  return error(fs_close(fd));
}

static int write_match(FILE *fp, struct match const *m)
{
  char buff[IMM_STATE_NAME_SIZE + 20] = {0};

  char *ptr = buff;
  struct imm_seq seq = match_subsequence(m);
  memcpy(ptr, imm_seq_data(&seq), (unsigned)imm_seq_size(&seq));
  ptr += imm_seq_size(&seq);
  *ptr = ',';
  ptr += 1;

  int rc = match_state_name(m, ptr);
  if (rc) return rc;
  ptr += strlen(ptr);
  *ptr = ',';
  ptr += 1;

  if (!match_state_is_mute(m))
  {
    struct imm_codon codon = {0};
    if ((rc = match_codon(m, &codon))) return rc;
    *ptr++ = imm_codon_asym(&codon);
    *ptr++ = imm_codon_bsym(&codon);
    *ptr++ = imm_codon_csym(&codon);
  }
  *ptr++ = ',';

  if (!match_state_is_mute(m))
  {
    char amino = 0;
    if ((rc = match_amino(m, &amino))) return rc;
    *ptr++ = amino;
  }
  *ptr = '\0';

  return fputs(buff, fp) == EOF ? error(DCP_EFWRITE) : 0;
}
