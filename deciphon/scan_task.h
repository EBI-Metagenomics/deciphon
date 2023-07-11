#ifndef DECIPHON_SCAN_TASK_H
#define DECIPHON_SCAN_TASK_H

#include "imm/imm.h"

struct dcp_seq;

struct dcp_scan_task
{
  struct imm_task *task;
  struct imm_prod prod;
};

void dcp_scan_task_init(struct dcp_scan_task *);
int dcp_scan_task_setup(struct dcp_scan_task *, struct imm_dp const *,
                        struct dcp_seq const *);
void dcp_scan_task_cleanup(struct dcp_scan_task *);

#endif
