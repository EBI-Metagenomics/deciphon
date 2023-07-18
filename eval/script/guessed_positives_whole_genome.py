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
from read_nucltdb import compute_genome_size, read_nucltdb
from whole_genome_index import WholeGenomeIndex

from deciphon_eval.location import Location
from deciphon_eval.portion_utils import discretize


def guessed_positives_whole_genome(genome_dir: str, dbfile: str):
    nucltdb = read_nucltdb(Path(genome_dir) / "cds_from_genomic.fna")
    index = WholeGenomeIndex(ProfileIndex(str(dbfile)), compute_genome_size(nucltdb))
    snap = read_snap(Path(genome_dir) / "cds_from_genomic.dcs")

    gps = []
    for prod in snap.products:
        x = nucltdb.row(by_predicate=pl.col("seqidx") == prod.seq_id, named=True)
        loc = Location.from_string(x["nuclt_location"])
        gp = portion.empty()
        qibuilder = QueryIntervalBuilder(prod.match_list)
        for hit in HitList.make(prod.match_list):
            qi = qibuilder.make(hit.match_list_interval)
            offset = qi.pyinterval.start
            for x in prod.match_list[hit.match_list_interval.slice]:
                if x.state.startswith("I"):
                    offset += len(x.query)
                if x.state.startswith("M"):
                    base_idx = loc.project(offset) - 1
                    lower = index.index(prod.profile, base_idx)
                    upper = index.index(prod.profile, base_idx + len(x.query) - 1)
                    gp |= portion.closed(lower, upper)
                    offset += len(x.query)
        gps.append(discretize(gp))
    guessed_positives = reduce(portion.Interval.union, gps, portion.empty())
    guessed_positives = discretize(guessed_positives)

    with open(Path(genome_dir) / "guessed_positives.pkl", "wb") as f:
        pkl.dump(guessed_positives, f)


if __name__ == "__main__":
    fire.Fire(guessed_positives_whole_genome)
