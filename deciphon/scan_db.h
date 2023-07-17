#ifndef DECIPHON_SCAN_DB_H
#define DECIPHON_SCAN_DB_H

#include "db_reader.h"
#include "protein_reader.h"
#include "size.h"

struct dcp_scan_db
{
  FILE *fp;
  struct dcp_db_reader db;
  struct dcp_protein_reader rdr;
};

struct imm_abc;

void dcp_scan_db_init(struct dcp_scan_db *);
int dcp_scan_db_open(struct dcp_scan_db *, char const *filename, int nthreads);
void dcp_scan_db_close(struct dcp_scan_db *);

struct dcp_protein_reader *dcp_scan_db_reader(struct dcp_scan_db *);
struct imm_code const *dcp_scan_code(struct dcp_scan_db const *);

#endif
