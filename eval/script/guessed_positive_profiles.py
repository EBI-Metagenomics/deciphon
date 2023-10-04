import pickle as pkl
from pathlib import Path

import fire
import polars as pl
from deciphon_snap.read_snap import read_snap
from profile_index import ProfileIndex
from sequence_profile_index import SequenceProfileIndex
from read_nucltdb import read_nucltdb
from sequence_index import SequenceIndex


def guessed_positive_profiles(genome_dir: str, pfam_file: str):
    fasta = Path(genome_dir) / "cds_from_genomic.fna"

    print("Reading nucleotides file...", end=" ", flush=True)
    nucltdb = read_nucltdb(fasta)
    print("done.")

    print("Generating sequence indices...", end=" ", flush=True)
    sequence_index = SequenceIndex(str(fasta))
    print("done.")

    print("Generating profile indices...", end=" ", flush=True)
    profile_index = ProfileIndex(pfam_file)
    print("done.")

    pair = SequenceProfileIndex(sequence_index, profile_index)

    print("Reading products...", end=" ", flush=True)
    snap = read_snap(Path(genome_dir) / "cds_from_genomic.dcs")
    print("done.")

    print("Generating solution indices...", end=" ", flush=True)
    guess = []
    for prod in snap.products:
        x = nucltdb.row(by_predicate=pl.col("seqidx") == prod.seq_id, named=True)
        guess.append(pair.index(x["seqid"], prod.profile))
    print("done.")

    output = Path(genome_dir) / "guessed_positive_profiles.pkl"
    print(f"Writting {output}...", end=" ", flush=True)
    with open(output, "wb") as f:
        pkl.dump(set(guess), f)
    print("done.")


if __name__ == "__main__":
    fire.Fire(guessed_positive_profiles)
