#include "error.h"
#include "array_size.h"
#include "deciphon.h"
#include "loglevel.h"
#include "sink.h"
#include <stdio.h>

static char const *msg[] = {
    [DCP_EDIFFABC] = "different alphabets",
    [DCP_EFCLOSE] = "failed to close file",
    [DCP_EFDATA] = "invalid file data",
    [DCP_EREFOPEN] = "failed to re-open file",
    [DCP_EFREAD] = "failed to read from file",
    [DCP_EFSEEK] = "failed to seek file",
    [DCP_EFTELL] = "failed to get file position",
    [DCP_EFUNCUSE] = "invalid function usage",
    [DCP_EFWRITE] = "failed to write to file",
    [DCP_EGETPATH] = "failed to get file path",
    [DCP_EZEROSEQ] = "zero-length sequence",
    [DCP_EZEROMODEL] = "zero-length model",
    [DCP_EZEROPART] = "no partition",
    [DCP_EDECODON] = "failed to decode into codon",
    [DCP_ELARGEMODEL] = "model is too large",
    [DCP_ELARGEPROTEIN] = "protein is too large",
    [DCP_EREADHMMER3] = "failed to read hmmer3 profile",
    [DCP_EMANYPARTS] = "too may partitions",
    [DCP_EMANYTRANS] = "too many transitions",
    [DCP_ENOMEM] = "not enough memory",
    [DCP_EOPENDB] = "failed to open DB file",
    [DCP_EOPENHMM] = "failed to open HMM file",
    [DCP_EOPENTMP] = "failed to open temporary file",
    [DCP_ETRUNCPATH] = "truncated file path",
    [DCP_EDPUNPACK] = "failed to unpack DP",
    [DCP_EDPPACK] = "failed to pack DP",
    [DCP_ENUCLTDUNPACK] = "failed to unpack nuclt dist",
    [DCP_ENUCLTDPACK] = "failed to pack nuclt dist",
    [DCP_ESETTRANS] = "failed to set transition",
    [DCP_EADDSTATE] = "failed to add state",
    [DCP_EDPRESET] = "failed to reset DP",
    [DCP_EFSTAT] = "failed to get file stat",
    [DCP_EFOPEN] = "failed to open file",
    [DCP_ELARGEFILE] = "file is too large",
    [DCP_ELONGPATH] = "path is too long",
    [DCP_EIMMRESETTASK] = "failed to reset task",
    [DCP_EIMMNEWTASK] = "failed to create new task",
    [DCP_EIMMSETUPTASK] = "failed to setup task",
    [DCP_EWRITEPROD] = "failed to write product",
    [DCP_EINVALPART] = "invalid partition",
    [DCP_ELONGACCESSION] = "accession string is too long",
    [DCP_EMANYTHREADS] = "too many threads",
    [DCP_ETMPFILE] = "failed to create temporary file",
    [DCP_EFFLUSH] = "failed to flush file",
    [DCP_EMKDIR] = "failed to create directory",
    [DCP_EFORMAT] = "wrong file format",
    [DCP_ERMDIR] = "failed to remove directory",
    [DCP_ERMFILE] = "failed to remove file",
    [DCP_ESETGENCODE] = "must set gencode first",
    [DCP_EGENCODEID] = "invalid gencode id",
    [DCP_EH3CDIAL] = "dialing to hmmer daemon failed",
    [DCP_EH3CPUT] = "failed to put a task to the hmmer daemon",
    [DCP_EH3CPOP] = "failed to pop a task from the hmmer daemon",
    [DCP_EH3CPACK] = "failed to pack hmmer result",
    [DCP_EH3CMAXRETRY] = "reached maximum number of retries on hmmer daemon",
    [DCP_EH3CWARMUP] = "failed to warmup hmmer daemon",
    [DCP_ESEQABC] = "invalid sequence letter (neither DNA nor RNA alphabet)",
    [DCP_EFDOPEN] = "failed to open file descriptor",
    [DCP_EMKSTEMP] = "failed to make temporary file",
    [DCP_ELONGABC] = "abc string is too long",
    [DCP_ELONGCONSENSUS] = "consensus string is too long",
    [DCP_ELARGECORESIZE] = "number of core nodes is too long",
    [DCP_EINVALSTATE] = "invalid state",
    [DCP_EINVALSIZE] = "invalid size",
    [DCP_EENDOFFILE] = "unexpected end of file",
    [DCP_EENDOFNODES] = "unexpected end of nodes",
    [DCP_EDBVERSION] = "unsupported database version",
    [DCP_ENOTDBFILE] = "not a database file",
    [DCP_EINVALSTATEID] = "invalid state id",
    [DCP_ENUCLTNOSUPPORT] = "unsupported nucleotide (must be either DNA or RNA)",
    [DCP_EDBDNASEQRNA] = "database is DNA but sequence is RNA",
    [DCP_EDBRNASEQDNA] = "database is RNA but sequence is DNA",
    [DCP_ENUCLTSEQTU] = "nucleotide sequence cannot have both U and T",
    [DCP_ENOHIT] = "failed to find hit",
    [DCP_EOPEN] = "failed to open file",
    [DCP_ECLOSE] = "failed to close file",
    [DCP_EDUP] = "failed to duplicate descriptor",
    [DCP_ETOOMANYPROTEINS] = "too many proteins",
    [DCP_EINVALNUMPROTEINS]= "invalid number of proteins",
};

char const *dcp_error_string(int error_code)
{
  if (error_code > 0 && error_code < (int)array_size(msg))
    return msg[error_code];

  static char unknown[32] = {0};
  snprintf(unknown, sizeof unknown, "unknown error #%d", error_code);
  return unknown;
}

int error_raise(int error_code, int line, const char *file, const char *func)
{
  if (loglevel() <= LOGLEVEL_ERROR)
    fprintf(SINK_ERROR, "%s:%d:%s: %s\n", file, line, func,
            dcp_error_string(error_code));
  return error_code;
}
