from pathlib import Path

import polars as pl

from deciphon_eval.location import Location
from deciphon_eval.portion_utils import interval_upper

__all__ = ["read_nucltdb", "compute_genome_size"]


def read_nucltdb(filename: Path):
    schema = {"seqid": pl.Utf8, "nuclt_location": pl.Utf8}
    data = {k: [] for k in schema.keys()}

    for row in open(filename, "r"):
        if row.startswith(">"):
            name = row[1:].strip().split(" ", 1)[0]
            data["seqid"].append(name)
            nuclt_location = Location.from_defline(row)
            data["nuclt_location"].append(str(nuclt_location))

    df = pl.DataFrame(data, schema=schema)
    df = df.with_columns(pl.col("seqid").str.replace("_cds_", "_any_"))

    seqidx = pl.Series(name="seqidx", values=list(range(df.shape[0])), dtype=pl.Int64)
    df = df.with_columns(seqidx)

    cols = [
        pl.col("seqidx"),
        pl.col("seqid"),
        pl.col("nuclt_location"),
    ]
    df = df.select(cols)
    return df


def compute_genome_size(df: pl.DataFrame):
    genome_size = 0
    for x in df.get_column("nuclt_location"):
        y = max(interval_upper(i) for i in Location.from_string(x).intervals)
        genome_size = max(genome_size, y)
    return genome_size


if __name__ == "__main__":
    root = Path("/Users/horta/tune-deciphon/phase3/0")
    organism = "GCF_013365495.1_ASM1336549v1"
    file = root / f"bacteria/{organism}/cds_from_genomic.fna"
    nucltdb = read_nucltdb(file)
    print(nucltdb)
    print(compute_genome_size(nucltdb))
