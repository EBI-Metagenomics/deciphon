import pickle as pkl
from functools import reduce
from pathlib import Path

import fire
import polars as pl
import portion
from deciphon_snap.hit import HitList
from deciphon_snap.query_interval import QueryIntervalBuilder
from deciphon_snap.read_snap import read_snap
from profile_index import ProfileIndex
from profile_nucleotide_index import ProfileNucleotideIndex
from read_nucltdb import compute_genome_size, read_nucltdb

from deciphon_eval.location import Location
from deciphon_eval.portion_utils import discretize


def guessed_positive_nucleotides(genome_dir: str, pfam_file: str):
    fasta = Path(genome_dir) / "cds_from_genomic.fna"

    print("Reading nucleotides file...", end=" ", flush=True)
    nucltdb = read_nucltdb(fasta)
    print("done.")

    print("Generating profile indices...", end=" ", flush=True)
    profile_index = ProfileIndex(pfam_file)
    print("done.")

    pair = ProfileNucleotideIndex(profile_index, compute_genome_size(nucltdb))

    print("Reading products...", end=" ", flush=True)
    snap = read_snap(Path(genome_dir) / "cds_from_genomic.dcs")
    print("done.")

    print("Generating solution indices...", end=" ", flush=True)
    gps = []
    for prod in snap.products:
        x = nucltdb.row(by_predicate=pl.col("seqidx") == prod.seq_id, named=True)
        loc = Location.from_string(x["nuclt_location"])
        gp = portion.empty()
        qibuilder = QueryIntervalBuilder(prod.match_list.evaluate())
        for hit in HitList.make(prod.match_list.evaluate()):
            qi = qibuilder.make(hit.match_list_interval)
            offset = qi.pyinterval.start
            for x in prod.match_list[hit.match_list_interval.slice]:
                if x.state.startswith("I"):
                    offset += len(x.query)
                if x.state.startswith("M"):
                    base_idx = loc.project(offset) - 1
                    profile = prod.profile
                    lower = pair.index(profile, base_idx)
                    upper = pair.index(profile, base_idx + len(x.query) - 1)
                    gp |= portion.closed(lower, upper)
                    offset += len(x.query)
        gps.append(discretize(gp))
    guess = reduce(portion.Interval.union, gps, portion.empty())
    guess = discretize(guess)
    print("done.")

    output = Path(genome_dir) / "guessed_positive_nucleotides.pkl"
    print(f"Writting {output}...", end=" ", flush=True)
    with open(output, "wb") as f:
        pkl.dump(guess, f)
    print("done.")


if __name__ == "__main__":
    fire.Fire(guessed_positive_nucleotides)
