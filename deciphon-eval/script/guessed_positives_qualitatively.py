import pickle as pkl
from pathlib import Path

import fire
import polars as pl
from deciphon_snap.read_snap import read_snap
from profile_index import ProfileIndex
from qual_index import QualIndex
from read_nucltdb import read_nucltdb
from sequence_index import SequenceIndex


def guessed_positives_qualitatively(genome_dir: str, dbfile: str):
    nucltdb = read_nucltdb(Path(genome_dir) / "cds_from_genomic.fna")
    fasta = Path(genome_dir) / "cds_from_genomic.fna"
    index = QualIndex(SequenceIndex(str(fasta)), ProfileIndex(dbfile))
    snap = read_snap(Path(genome_dir) / "cds_from_genomic.dcs")

    guess = []
    for prod in snap.products:
        x = nucltdb.row(by_predicate=pl.col("seqidx") == prod.seq_id, named=True)
        guess.append(index.index(x["seqid"], prod.profile))

    with open(Path(genome_dir) / "guessed_positives_qual.pkl", "wb") as f:
        pkl.dump(set(guess), f)


if __name__ == "__main__":
    fire.Fire(guessed_positives_qualitatively)
