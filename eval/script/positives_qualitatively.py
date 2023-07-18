import pickle as pkl
from collections import defaultdict
from pathlib import Path

import fire
import polars as pl
from hmmer_results import read_hmmer_results
from profile_index import ProfileIndex
from qual_index import QualIndex
from read_nucltdb import read_nucltdb
from sequence_index import SequenceIndex


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


def positives_qualitatively(genome_dir: str, dbfile: str):
    hmmer = read_hmmer_results(Path(genome_dir))
    fasta = Path(genome_dir) / "cds_from_genomic.fna"
    nucltdb = read_nucltdb(fasta)
    index = QualIndex(SequenceIndex(str(fasta)), ProfileIndex(dbfile))

    positives = []
    for seqrow in nucltdb.rows(named=True):
        seqid = seqrow["seqid"]
        aligns_dict = get_aligns(hmmer.output, seqid)
        for profname, aligns in aligns_dict.items():
            col = pl.col("profile_name")
            t = hmmer.domtbl.filter(col == profname).rows(named=True)[0]
            profid = t["profid"]
            j = index.index(seqid, profid)
            positives.append(j)

    with open(Path(genome_dir) / "positives_qual.pkl", "wb") as f:
        pkl.dump(set(positives), f)


if __name__ == "__main__":
    fire.Fire(positives_qualitatively)
