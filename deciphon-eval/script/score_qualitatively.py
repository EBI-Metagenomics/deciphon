import sys
from pathlib import Path
from profile_index import ProfileIndex

import polars as pl
from deciphon_snap.read_snap import read_snap
from hmmer_tables.output import read_output

from deciphon_eval.confusion import ConfusionMatrix
from load_domtbl import load_domtbl
from load_nuclt_db import load_nuclt_db


genome_dir = Path(sys.argv[1])
dbhmm = Path(sys.argv[2])

snap = read_snap(genome_dir / "cds_from_genomic.dcs")
domtbl = load_domtbl(genome_dir / "domtbl.txt")
nuclt = load_nuclt_db(genome_dir / "cds_from_genomic.fna")
output = read_output(genome_dir / "output.txt")


class SequenceIndex:
    def __init__(self, nuclt):
        seqs = [str(x[0]) for x in nuclt.df.select(pl.col("seq_name_any")).rows()]
        self._index = {}
        for i, x in enumerate(set(seqs)):
            self._index[x] = i

    @property
    def count(self):
        return len(self._index)

    def index(self, seq_name: str):
        return self._index[seq_name]

    def keys(self):
        return self._index.keys()


class Index:
    def __init__(self, seq_idx: SequenceIndex, prof_idx: ProfileIndex):
        self._seq_idx = seq_idx
        self._prof_idx = prof_idx

    @property
    def count(self):
        return self._seq_idx.count * self._prof_idx.count

    def index(self, seq_name: str, profile: str):
        n = self._prof_idx.count
        return self._seq_idx.index(seq_name) * n + self._prof_idx.index(profile)


seq_idx = SequenceIndex(nuclt)
prof_idx = ProfileIndex(str(dbhmm))
index = Index(seq_idx, prof_idx)

P = []
for row in domtbl.df.select([pl.col("seq_name"), pl.col("profile")]).rows(named=True):
    P.append(index.index(row["seq_name"], row["profile"]))
P = set(P)

guessP = []
for prod in snap.products:
    seq_name = nuclt.seq_name(prod.seq_id).replace("_cds_", "")
    guessP.append(index.index(seq_name, prod.profile))
guessP = set(guessP)

TP = P & guessP
FP = P - TP

P = len(P)
TP = len(TP)
FP = len(FP)
N = index.count - P

cm = ConfusionMatrix(P=P, N=N, TP=TP, FP=FP)
print(f"Precision={cm.precision},Recall={cm.recall}")
