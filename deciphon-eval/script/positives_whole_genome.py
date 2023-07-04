import pickle as pkl
import sys
from collections import defaultdict
from functools import reduce
from pathlib import Path

import fire
import polars as pl
import portion
from hmmer_results import read_hmmer_results
from profile_index import ProfileIndex
from read_nucltdb import compute_genome_size, read_nucltdb
from tqdm import tqdm
from whole_genome_index import Index

from deciphon_eval.location import Location
from deciphon_eval.portion_utils import discretize


def get_aligns(output, seqid: str):
    queries = []
    for query in output.queries:
        assert query.head.split("\n")[0][:6] == "Query:"
        x = query.head.split("\n")[0][6:].strip()
        stop = x.rindex(" ") - 1
        x = x[:stop].strip()
        if x.replace("_prot_", "_any_") == seqid:
            queries.append(query)
    if len(queries) == 0:
        raise ValueError(f"seq_name {seqid} not found.")
    assert len(queries) == 1
    query = queries[0]
    aligns = defaultdict(list)
    for dom in query.domains:
        for align in dom.aligns:
            aligns[align.align.profile].append(align.align)
    return aligns


def positives_whole_genome(genome_dir: Path, db_dir: Path, silent: bool = False):
    print(sys.argv)
    genome_dir = Path(sys.argv[1])
    db_dir = Path(sys.argv[2])
    hmmer = read_hmmer_results(genome_dir)
    nucltdb = read_nucltdb(genome_dir / "cds_from_genomic.fna")
    index = Index(ProfileIndex(str(db_dir)), compute_genome_size(nucltdb))

    ps = []
    for seqrow in tqdm(nucltdb.rows(named=True), disable=silent):
        seqid = seqrow["seqid"]
        loc = Location.from_string(seqrow["nuclt_location"])
        aligns_dict = get_aligns(hmmer.output, seqid)
        for profname, aligns in aligns_dict.items():
            p = portion.empty()
            col = pl.col("profile_name")
            t = hmmer.domtbl.filter(col == profname).rows(named=True)[0]
            profid = t["profid"]
            for x in aligns:
                offset = x.query_interval.to_pyinterval().start * 3
                for cs, q in zip(x.query_cs, x.query):
                    if cs == "." and q != "-":
                        # INSERT
                        offset += 3
                    if cs != "." and q == "-":
                        # DELETE
                        pass
                    if cs != "." and q != "-":
                        # MATCH
                        base_idx = loc.project(offset) - 1
                        j = index.index(profid, base_idx)
                        p |= portion.closed(j, j + 2)
                        offset += 3
            ps.append(discretize(p))
    positives = reduce(portion.Interval.union, ps, portion.empty())
    positives = discretize(positives)

    with open(genome_dir / "positives.pkl", "wb") as f:
        pkl.dump(positives, f)


if __name__ == "__main__":
    fire.Fire(positives_whole_genome)
