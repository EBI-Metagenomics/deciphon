#ifndef DECIPHON_SCAN_DB_H
#define DECIPHON_SCAN_DB_H

#include "db_reader.h"
#include "protein_reader.h"
#include <stdio.h>

struct dcp_scan_db
{
  char filename[FILENAME_MAX];
  FILE *fp;
  struct dcp_db_reader db;
  struct dcp_protein_reader rdr;
};

struct imm_abc;

void dcp_scan_db_init(struct dcp_scan_db *);
int dcp_scan_db_open(struct dcp_scan_db *, int nthreads);
void dcp_scan_db_close(struct dcp_scan_db *);

int dcp_scan_db_set_filename(struct dcp_scan_db *, char const *);
struct dcp_protein_reader *dcp_scan_db_reader(struct dcp_scan_db *);
struct imm_abc const *dcp_scan_db_abc(struct dcp_scan_db const *);

#endif
