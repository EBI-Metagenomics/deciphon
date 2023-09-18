#include "deciphon/db_reader.h"
#include "deciphon/db_writer.h"
#include "deciphon/protein_iter.h"
#include "deciphon/protein_reader.h"
#include "imm/imm.h"
#include "vendor/minctest.h"

static void test_protein_db_writer(void);
static void test_protein_db_reader(void);

int main(void)
{
  test_protein_db_writer();
  test_protein_db_reader();
  return lfails;
}

static void test_protein_db_writer(void)
{
  remove("db.dcp");

  struct imm_amino const *amino = &imm_amino_iupac;
  struct imm_nuclt const *nuclt = &imm_dna_iupac.super;
  struct imm_nuclt_code code = {};
  imm_nuclt_code_init(&code, nuclt);

  FILE *fp = fopen("db.dcp", "wb");
  ok(fp);

  struct dcp_model_params params = {imm_gencode_get(1), amino, &code,
                                    DCP_ENTRY_DIST_OCCUPANCY, 0.01};

  struct dcp_db_writer db = {};
  dcp_db_writer_init(&db, params);
  eq(dcp_db_writer_open(&db, fp), 0);

  struct dcp_protein protein = {};
  struct p7 p7 = {};
  dcp_protein_init(&protein, params);
  p7_init(&p7, params);
  dcp_protein_set_accession(&protein, "accession0");
  p7_set_accession(&p7, "accession0");

  unsigned core_size = 2;
  dcp_protein_sample(&protein, 1, core_size);
  p7_sample(&p7, 1, core_size);
  eq(dcp_db_writer_pack(&db, &protein, &p7), 0);

  dcp_protein_sample(&protein, 2, core_size);
  p7_sample(&p7, 2, core_size);
  eq(dcp_db_writer_pack(&db, &protein, &p7), 0);

  p7_cleanup(&p7);
  dcp_protein_cleanup(&protein);
  eq(dcp_db_writer_close(&db), 0);
  fclose(fp);
}

static char const seqchars[] =
    "AAAACGCGTGTCACGACAACGCGTACGTTTCGACGAGTACGACGCCCGGG"
    "AAAACGCGTGTCGACGACGAACGCGTACGTTTACGACGAGTACGACGCCC"
    "AAAACGCGTGTCACGACAACGCGTACGTTTCGACGAGTACGACGCCCGGG"
    "AAAACGCGTGTCGACGACGAACGCGTACGTTTACGACGAGTACGACGCCC"
    "AAAACGCGTGTCACGACAACGCGTACGTTTCGACGAGTACGACGCCCGGG"
    "AAAACGCGTGTCGACGACGAACGCGTACGTTTACGACGAGTACGACGCCC"
    "AAAACGCGTGTCACGACAACGCGTACGTTTCGACGAGTACGACGCCCGGG"
    "AAAACGCGTGTCGACGACGAACGCGTACGTTTACGACGAGTACGACGCCC"
    "AAAACGCGTGTCACGACAACGCGTACGTTTCGACGAGTACGACGCCCGGG"
    "AAAACGCGTGTCGACGACGAACGCGTACGTTTACGACGAGTACGACGCCC"
    "AAAACGCGTGTCACGACAACGCGTACGTTTCGACGAGTACGACGCCCGGG"
    "AAAACGCGTGTCGACGACGAACGCGTACGTTTACGACGAGTACGACGCCC"
    "AAAACGCGTGTCACGACAACGCGTACGTTTCGACGAGTACGACGCCCGGG"
    "AAAACGCGTGTCGACGACGAACGCGTACGTTTACGACGAGTACGACGCCC"
    "AAAACGCGTGTCACGACAACGCGTACGTTTCGACGAGTACGACGCCCGGG"
    "AAAACGCGTGTCGACGACGAACGCGTACGTTTACGACGAGTACGACGCCC"
    "AAAACGCGTGTCACGACAACGCGTACGTTTCGACGAGTACGACGCCCGGG"
    "AAAACGCGTGTCGACGACGAACGCGTACGTTTACGACGAGTACGACGCCC"
    "AAAACGCGTGTCACGACAACGCGTACGTTTCGACGAGTACGACGCCCGGG"
    "AAAACGCGTGTCGACGACGAACGCGTACGTTTACGACGAGTACGACGCCC"
    "AAAACGCGTGTCACGACAACGCGTACGTTTCGACGAGTACGACGCCCGGG"
    "AAAACGCGTGTCGACGACGAACGCGTACGTTTACGACGAGTACGACGCCC"
    "AAAACGCGTGTCACGACAACGCGTACGTTTCGACGAGTACGACGCCCGGG"
    "AAAACGCGTGTCGACGACGAACGCGTACGTTTACGACGAGTACGACGCCC"
    "AAAACGCGTGTCACGACAACGCGTACGTTTCGACGAGTACGACGCCCGGG"
    "AAAACGCGTGTCGACGACGAACGCGTACGTTTACGACGAGTACGACGCCC"
    "AAAACGCGTGTCACGACAACGCGTACGTTTCGACGAGTACGACGCCCGGG"
    "AAAACGCGTGTCGACGACGAACGCGTACGTTTACGACGAGTACGACGCCC"
    "AAAACGCGTGTCACGACAACGCGTACGTTTCGACGAGTACGACGCCCGGG"
    "AAAACGCGTGTCGACGACGAACGCGTACGTTTACGACGAGTACGACGCCC"
    "AAAACGCGTGTCACGACAACGCGTACGTTTCGACGAGTACGACGCCCGGG"
    "AAAACGCGTGTCGACGACGAACGCGTACGTTTACGACGAGTACGACGCCC"
    "AAAACGCGTGTCACGACAACGCGTACGTTTCGACGAGTACGACGCCCGGG"
    "AAAACGCGTGTCGACGACGAACGCGTACGTTTACGACGAGTACGACGCCC"
    "AAAACGCGTGTCACGACAACGCGTACGTTTCGACGAGTACGACGCCCGGG"
    "AAAACGCGTGTCGACGACGAACGCGTACGTTTACGACGAGTACGACGCCC"
    "AAAACGCGTGTCACGACAACGCGTACGTTTCGACGAGTACGACGCCCGGG"
    "AAAACGCGTGTCGACGACGAACGCGTACGTTTACGACGAGTACGACGCCC"
    "AAAACGCGTGTCACGACAACGCGTACGTTTCGACGAGTACGACGCCCGGG"
    "AAAACGCGTGTCGACGACGAACGCGTACGTTTACGACGAGTACGACGCCC";

static void test_protein_db_reader(void)
{
  FILE *fp = fopen("db.dcp", "rb");
  ok(fp);
  struct dcp_db_reader db = {0};
  eq(dcp_db_reader_open(&db, fp), 0);

  eq(db.nproteins, 2);

  struct imm_abc const *abc = &db.nuclt.super;
  eq((int)abc->typeid, IMM_DNA);

  struct imm_eseq eseq = {0};
  imm_eseq_init(&eseq, &db.code.super);

  double logliks[] = {-2720.381428394979, -2849.83007812500};

  unsigned nproteins = 0;
  struct imm_prod prod = imm_prod();
  int rc = 0;
  struct dcp_protein_reader reader = {0};
  eq(dcp_protein_reader_setup(&reader, &db, 1), 0);
  struct dcp_protein_iter it = {0};
  eq(dcp_protein_reader_iter(&reader, 0, &it), 0);

  struct imm_gencode const *gencode = imm_gencode_get(1);

  struct dcp_protein protein = {};
  struct p7 p7 = {};
  dcp_protein_init(&protein, dcp_protein_reader_params(&reader, gencode));
  p7_init(&p7, dcp_protein_reader_params(&reader, gencode));
  while (!(rc = dcp_protein_iter_next(&it, &protein, &p7)))
  {
    if (dcp_protein_iter_end(&it)) break;

    struct imm_task *task = imm_task_new(&protein.alts.full.dp);
    struct imm_seq seq = imm_seq(imm_str(seqchars), abc);
    eq(imm_eseq_setup(&eseq, &seq), 0);
    eq(imm_task_setup(task, &eseq), 0);
    eq(imm_dp_viterbi(&protein.alts.full.dp, task, &prod), 0);
    close(prod.loglik, logliks[nproteins]);
    imm_task_del(task);
    ++nproteins;
  }
  eq(rc, 0);
  eq(nproteins, 2U);

  imm_eseq_cleanup(&eseq);
  imm_prod_cleanup(&prod);
  dcp_protein_cleanup(&protein);
  p7_cleanup(&p7);
  dcp_db_reader_close(&db);
  fclose(fp);
  remove("db.dcp");
}
