#ifndef INFER_AMINO_H
#define INFER_AMINO_H

struct chararray;
struct match;
struct match_iter;

int infer_amino(struct chararray *, struct match *, struct match_iter *);

#endif
