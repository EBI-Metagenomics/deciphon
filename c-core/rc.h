#ifndef DECIPHON_RC_H
#define DECIPHON_RC_H

#include "api.h"

// clang-format off
enum dcp_rc
{
  DCP_EDIFFABC      = 1,
  DCP_EFCLOSE       = 2,
  DCP_EFDATA        = 3,
  DCP_EREFOPEN      = 4,
  DCP_EFREAD        = 5,
  DCP_EFSEEK        = 6,
  DCP_EFTELL        = 7,
  DCP_EFUNCUSE      = 8,
  DCP_EFWRITE       = 9,
  DCP_EGETPATH      = 10,
  DCP_EZEROSEQ      = 11,
  DCP_EZEROMODEL    = 12,
  DCP_EZEROPART     = 13,
  DCP_EDECODON      = 14,
  DCP_ELARGEMODEL   = 15,
  DCP_ELARGEPROTEIN = 16,
  DCP_EREADHMMER3   = 17,
  DCP_EMANYPARTS    = 18,
  DCP_EMANYTRANS    = 19,
  DCP_ENOMEM        = 20,
  DCP_EOPENDB       = 21,
  DCP_EOPENHMM      = 22,
  DCP_EOPENTMP      = 23,
  DCP_ETRUNCPATH    = 24,
  DCP_EDPUNPACK     = 25,
  DCP_EDPPACK       = 26,
  DCP_ENUCLTDUNPACK = 27,
  DCP_ENUCLTDPACK   = 28,
  DCP_ESETTRANS     = 29,
  DCP_EADDSTATE     = 30,
  DCP_EDPRESET      = 31,
  DCP_EFSTAT        = 32,
  DCP_EFOPEN        = 33,
  DCP_ELARGEFILE    = 34,
  DCP_ELONGPATH     = 35,
  DCP_EIMMRESETTASK = 36,
  DCP_EIMMNEWTASK   = 37,
  DCP_EIMMSETUPTASK = 38,
  DCP_EWRITEPROD    = 39,
  DCP_EINVALPART    = 40,
  DCP_ELONGACC      = 41,
  DCP_EMANYTHREADS  = 42,
  DCP_ETMPFILE      = 43,
  DCP_EFFLUSH       = 44,
  DCP_EMKDIR        = 45,
  DCP_EFORMAT       = 46,
  DCP_ERMDIR        = 47,
  DCP_ERMFILE       = 48,
  DCP_ESETGENCODE   = 49,
  DCP_EGENCODEID    = 50,
  DCP_EH3CDIAL      = 51,
  DCP_EH3CPUT       = 52,
  DCP_EH3CPOP       = 53,
  DCP_EH3CPACK      = 54,
  DCP_EH3CMAXRETRY  = 55,
  DCP_EH3CWARMUP    = 56,
  DCP_ESEQABC       = 57,
};
// clang-format on

#endif