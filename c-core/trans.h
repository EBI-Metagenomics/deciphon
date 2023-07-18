#ifndef DECIPHON_TRANS_H
#define DECIPHON_TRANS_H

enum
{
  TRANS_SIZE = 7
};

struct dcp_trans
{
  union
  {
    struct
    {
      float MM;
      float MI;
      float MD;
      float IM;
      float II;
      float DM;
      float DD;
    } __attribute__((packed));
    struct
    {
      float data[TRANS_SIZE];
    };
  };
};

#endif