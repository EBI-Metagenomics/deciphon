#include "aye.h"
#include "imm_dna.h"
#include "imm_gencode.h"
#include "imm_nuclt_code.h"
#include "imm_path.h"
#include "match.h"
#include "model_params.h"
#include "protein.h"
#include "state.h"

int main(void)
{
  aye_begin();
  struct imm_amino const *amino = &imm_amino_iupac;
  struct imm_nuclt const *nuclt = &imm_dna_iupac.super;
  struct imm_nuclt_code code = {};
  imm_nuclt_code_init(&code, nuclt);

  struct model_params params = {
      .gencode = imm_gencode_get(1),
      .amino = amino,
      .code = &code,
      .entry_dist = ENTRY_DIST_UNIFORM,
      .epsilon = 0.1,
  };

  struct protein protein = {0};
  protein_init(&protein);
  protein_setup(&protein, params);
  protein_set_accession(&protein, "accession");
  aye(protein_sample(&protein, 1, 10) == 0);

  char const str[] = "ATGAAACGCATTAGCACCACCATTACCACCAC";
  struct imm_seq seq = imm_seq_unsafe(imm_str(str), &nuclt->super);

  protein_reset(&protein, imm_seq_size(&seq), true, false);

  struct imm_path path = imm_path();
  aye(imm_path_add(&path, imm_step(STATE_S, 0, 0)) == 0);
  aye(imm_path_add(&path, imm_step(STATE_B, 0, 0)) == 0);
  aye(imm_path_add(&path, imm_step(state_make_match_id(0), 3, 0)) == 0);
  aye(imm_path_add(&path, imm_step(state_make_insert_id(0), 4, 0)) == 0);
  aye(imm_path_add(&path, imm_step(state_make_match_id(1), 2, 0)) == 0);
  aye(imm_path_add(&path, imm_step(STATE_E, 0, 0)) == 0);
  aye(imm_path_add(&path, imm_step(STATE_J, 3, 0)) == 0);
  aye(imm_path_add(&path, imm_step(STATE_B, 0, 0)) == 0);
  aye(imm_path_add(&path, imm_step(state_make_match_id(1), 3, 0)) == 0);
  aye(imm_path_add(&path, imm_step(state_make_delete_id(2), 0, 0)) == 0);
  aye(imm_path_add(&path, imm_step(state_make_match_id(3), 3, 0)) == 0);
  aye(imm_path_add(&path, imm_step(STATE_E, 0, 0)) == 0);
  aye(imm_path_add(&path, imm_step(STATE_B, 0, 0)) == 0);
  aye(imm_path_add(&path, imm_step(state_make_match_id(3), 5, 0)) == 0);
  aye(imm_path_add(&path, imm_step(state_make_match_id(4), 3, 0)) == 0);
  aye(imm_path_add(&path, imm_step(state_make_match_id(5), 3, 0)) == 0);
  aye(imm_path_add(&path, imm_step(state_make_match_id(6), 3, 0)) == 0);
  aye(imm_path_add(&path, imm_step(STATE_E, 0, 0)) == 0);
  aye(imm_path_add(&path, imm_step(STATE_T, 0, 0)) == 0);

  char aminos[] = {' ', ' ', 'D', 'N', 'A', ' ', 'I', ' ', 'S', ' ',
                   'T', ' ', ' ', 'N', 'Y', 'H', 'H', ' ', ' '};

  struct match it = match_begin(&path, &seq, &protein);
  int idx = 0;
  while (!match_equal(it, match_end()))
  {
    char name[256] = {0};
    aye(match_state_name(&it, name) == 0);
    char amino = 0;
    if (aminos[idx] != ' ')
    {
      aye(match_amino(&it, &amino) == 0);
      aye(aminos[idx] == amino);
    }
    it = match_next(&it);
    idx += 1;
  }

  struct match begin = match_end();
  struct match end = match_begin(&path, &seq, &protein);

  while (!match_equal(begin, end))
  {
    it = end;
    while (!match_equal(it, match_end()) && match_state_id(&it) != STATE_B)
      it = match_next(&it);

    begin = it;
    while (!match_equal(it, match_end()) && match_state_id(&it) != STATE_E)
      it = match_next(&it);
    end = match_next(&it);

    if (match_equal(begin, end)) continue;

    it = begin;
    while (!match_equal(it, end))
    {
      char name[256] = {0};
      aye(match_state_name(&it, name) == 0);
      char amino = 0;
      match_amino(&it, &amino);
      it = match_next(&it);
    }
  }

  imm_path_cleanup(&path);
  protein_cleanup(&protein);

  return aye_end();
}
