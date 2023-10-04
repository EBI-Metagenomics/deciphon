import pickle as pkl
from collections import defaultdict
from functools import reduce
from pathlib import Path

import fire
import polars as pl
import portion
from hmmer_results import read_hmmer_results
from profile_index import ProfileIndex
from profile_nucleotide_index import ProfileNucleotideIndex
from read_nucltdb import compute_genome_size, read_nucltdb

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


def positive_nuclts(genome_dir: str, pfam_file: str):
    fasta = Path(genome_dir) / "cds_from_genomic.fna"

    print("Reading hmmer results...", end=" ", flush=True)
    hmmer = read_hmmer_results(Path(genome_dir))
    print("done.")

    print("Generating profile indices...", end=" ", flush=True)
    profile_index = ProfileIndex(pfam_file)
    print("done.")

    print("Reading nucleotides file...", end=" ", flush=True)
    nucltdb = read_nucltdb(fasta)
    print("done.")

    pair = ProfileNucleotideIndex(profile_index, compute_genome_size(nucltdb))

    print("Generating solution indices...", end=" ", flush=True)
    ps = []
    for seqrow in nucltdb.rows(named=True):
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
                        j = pair.index(profid, base_idx)
                        p |= portion.closed(j, j + 2)
                        offset += 3
            ps.append(discretize(p))
    positives = reduce(portion.Interval.union, ps, portion.empty())
    positives = discretize(positives)
    print("done.")

    output = Path(genome_dir) / "true_positive_nucleotides.pkl"
    print(f"Writting {output}...", end=" ", flush=True)
    with open(output, "wb") as f:
        pkl.dump(positives, f)
    print("done.")


if __name__ == "__main__":
    fire.Fire(positive_nuclts)
