import pickle as pkl
from collections import defaultdict
from pathlib import Path

import fire
import polars as pl
from hmmer_results import read_hmmer_results
from profile_index import ProfileIndex
from read_nucltdb import read_nucltdb
from sequence_index import SequenceIndex
from sequence_profile_index import SequenceProfileIndex


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


def positive_profiles(genome_dir: str, pfam_file: str):
    fasta = Path(genome_dir) / "cds_from_genomic.fna"

    print("Reading hmmer results...", end=" ", flush=True)
    hmmer = read_hmmer_results(Path(genome_dir))
    print("done.")

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

    print("Generating solution indices...", end=" ", flush=True)
    positives = []
    for seqrow in nucltdb.rows(named=True):
        seqid = str(seqrow["seqid"])
        aligns_dict = get_aligns(hmmer.output, seqid)
        for profname, _ in aligns_dict.items():
            col = pl.col("profile_name")
            t = hmmer.domtbl.filter(col == profname).rows(named=True)[0]
            profid = str(t["profid"])
            j = pair.index(seqid, profid)
            positives.append(j)
    print("done.")

    output = Path(genome_dir) / "true_positive_profiles.pkl"
    print(f"Writting {output}...", end=" ", flush=True)
    with open(output, "wb") as f:
        pkl.dump(set(positives), f)
    print("done.")


if __name__ == "__main__":
    fire.Fire(positive_profiles)
