#include "hmm_reader.h"
#include "deciphon.h"
#include "error.h"
#include "model.h"
#include <math.h>

static void init_null_lprobs(float[IMM_AMINO_SIZE]);

int hmm_reader_init(struct hmm_reader *reader, struct model_params params,
                    FILE *fp)
{
  hmr_init(&reader->hmr, fp);
  hmr_profile_init(&reader->protein, &reader->hmr);
  init_null_lprobs(reader->null_lprobs);
  reader->end = false;
  return error(model_init(&reader->model, params, reader->null_lprobs));
}

int hmm_reader_next(struct hmm_reader *h3r)
{
  int hmr_rc = hmr_next_profile(&h3r->hmr, &h3r->protein);
  if (hmr_rc == HMR_EOF)
  {
    h3r->end = true;
    return 0;
  }

  if (hmr_rc) return error(DCP_EREADHMMER3);

  int core_size = (int)hmr_profile_length(&h3r->protein);
  int rc = 0;
  if ((rc = model_setup(&h3r->model, core_size))) return error(rc);
  h3r->model.has_ga = h3r->protein.meta.ga[0] != '\0';

  hmr_rc = hmr_next_node(&h3r->hmr, &h3r->protein);
  if (hmr_rc == HMR_EOF) return error(DCP_EENDOFFILE);

  struct trans t = {
      .MM = (float)h3r->protein.node.trans[HMR_TRANS_MM],
      .MI = (float)h3r->protein.node.trans[HMR_TRANS_MI],
      .MD = (float)h3r->protein.node.trans[HMR_TRANS_MD],
      .IM = (float)h3r->protein.node.trans[HMR_TRANS_IM],
      .II = (float)h3r->protein.node.trans[HMR_TRANS_II],
      .DM = (float)h3r->protein.node.trans[HMR_TRANS_DM],
      .DD = (float)h3r->protein.node.trans[HMR_TRANS_DD],
  };
  if ((rc = model_add_trans(&h3r->model, t))) return error(rc);

  while (!(hmr_rc = hmr_next_node(&h3r->hmr, &h3r->protein)))
  {
    float match_lprobs[IMM_AMINO_SIZE];
    for (int i = 0; i < IMM_AMINO_SIZE; ++i)
      match_lprobs[i] = (float)h3r->protein.node.match[i];

    char consensus = h3r->protein.node.excess.cons;
    if ((rc = model_add_node(&h3r->model, match_lprobs, consensus))) return error(rc);

    t = (struct trans){
        .MM = (float)h3r->protein.node.trans[HMR_TRANS_MM],
        .MI = (float)h3r->protein.node.trans[HMR_TRANS_MI],
        .MD = (float)h3r->protein.node.trans[HMR_TRANS_MD],
        .IM = (float)h3r->protein.node.trans[HMR_TRANS_IM],
        .II = (float)h3r->protein.node.trans[HMR_TRANS_II],
        .DM = (float)h3r->protein.node.trans[HMR_TRANS_DM],
        .DD = (float)h3r->protein.node.trans[HMR_TRANS_DD],
    };
    if ((rc = model_add_trans(&h3r->model, t))) return error(rc);
  }
  return hmr_rc == HMR_END ? 0 : error(DCP_EENDOFNODES);
}

bool hmm_reader_end(struct hmm_reader const *reader) { return reader->end; }

void hmm_reader_cleanup(struct hmm_reader *reader)
{
  model_cleanup(&reader->model);
}

static void init_null_lprobs(float lprobs[IMM_AMINO_SIZE])
{
  /* Copy/paste from HMMER3 amino acid frequences inferred from Swiss-Prot
   * 50.8, (Oct 2006), counting over 85956127 (86.0M) residues. */
  *(lprobs++) = logf(0.0787945); /*"A":*/
  *(lprobs++) = logf(0.0151600); /*"C":*/
  *(lprobs++) = logf(0.0535222); /*"D":*/
  *(lprobs++) = logf(0.0668298); /*"E":*/
  *(lprobs++) = logf(0.0397062); /*"F":*/
  *(lprobs++) = logf(0.0695071); /*"G":*/
  *(lprobs++) = logf(0.0229198); /*"H":*/
  *(lprobs++) = logf(0.0590092); /*"I":*/
  *(lprobs++) = logf(0.0594422); /*"K":*/
  *(lprobs++) = logf(0.0963728); /*"L":*/
  *(lprobs++) = logf(0.0237718); /*"M":*/
  *(lprobs++) = logf(0.0414386); /*"N":*/
  *(lprobs++) = logf(0.0482904); /*"P":*/
  *(lprobs++) = logf(0.0395639); /*"Q":*/
  *(lprobs++) = logf(0.0540978); /*"R":*/
  *(lprobs++) = logf(0.0683364); /*"S":*/
  *(lprobs++) = logf(0.0540687); /*"T":*/
  *(lprobs++) = logf(0.0673417); /*"V":*/
  *(lprobs++) = logf(0.0114135); /*"W":*/
  *(lprobs++) = logf(0.0304133); /*"Y":*/
};
