#include "scan_task.h"
#include "rc.h"
#include "seq.h"

void dcp_scan_task_init(struct dcp_scan_task *x) { x->task = NULL; }

int dcp_scan_task_setup(struct dcp_scan_task *x, struct imm_dp const *dp,
                        struct dcp_seq const *seq)
{
  if (x->task && imm_task_reset(x->task, dp)) return DCP_EIMMRESETTASK;
  if (!x->task && !(x->task = imm_task_new(dp))) return DCP_EIMMNEWTASK;
  if (imm_task_setup(x->task, dcp_seq_imm_eseq(seq))) return DCP_EIMMSETUPTASK;
  return 0;
}

void dcp_scan_task_cleanup(struct dcp_scan_task *x)
{
  if (x)
  {
    imm_task_del(x->task);
    x->task = NULL;
    imm_prod_cleanup(&x->prod);
  }
}
