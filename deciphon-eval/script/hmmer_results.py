from dataclasses import dataclass
from pathlib import Path

import polars as pl
from hmmer_tables.domtbl import read_domtbl
from hmmer_tables.output import Output, read_output

from deciphon_eval.rinterval import RInterval

__all__ = ["read_hmmer_results", "HMMERResults"]


@dataclass
class HMMERResults:
    domtbl: pl.DataFrame
    output: Output


def read_hmmer_results(directory: Path):
    return HMMERResults(
        domtbl=read_hmmer_domtbl(directory / "domtbl.txt"),
        output=read_output(directory / "output.txt"),
    )


def read_hmmer_domtbl(domtbl_file: Path) -> pl.DataFrame:
    schema = {
        "profid": pl.Utf8,
        "seqid": pl.Utf8,
        "amino_interval": pl.Utf8,
        "evalue": pl.Float64,
        "hmm_interval": pl.Utf8,
        "profile_name": pl.Utf8,
    }
    data = {k: [] for k in schema.keys()}

    for x in read_domtbl(domtbl_file):
        data["profid"].append(x.target.accession)
        amino_interval = RInterval.parse_obj(x.ali_coord.interval.to_rinterval())
        data["amino_interval"].append(str(amino_interval))
        data["evalue"].append(x.full_sequence.e_value)
        data["seqid"].append(x.query.name)
        hmm_interval = RInterval.parse_obj(x.hmm_coord.interval.to_rinterval())
        data["hmm_interval"].append(str(hmm_interval))
        data["profile_name"].append(x.target.name)

    df = pl.DataFrame(data, schema=schema)
    df = df.with_columns(pl.col("seqid").str.replace("_prot_", "_any_"))

    def build_nuclt_interval(x: dict):
        return str(RInterval.from_string(x["amino_interval"]).amino_to_nuclt())

    col = (
        pl.struct(["amino_interval"])
        .apply(build_nuclt_interval)
        .alias("nuclt_interval")
    )
    df = df.with_columns(col)

    cols = [
        "profid",
        "seqid",
        "nuclt_interval",
        "amino_interval",
        "evalue",
        "hmm_interval",
        "profile_name",
    ]
    return df.select([pl.col(x) for x in cols])


if __name__ == "__main__":
    root = Path("/Users/horta/tune-deciphon/phase3/0")
    organism = "GCF_013365495.1_ASM1336549v1"
    file = root / f"bacteria/{organism}/domtbl.txt"
    df = read_hmmer_domtbl(file)
    print(df)
    file = root / f"bacteria/{organism}/output.txt"
    output = read_output(file)
    assert output is not None
    results = read_hmmer_results(root / f"bacteria/{organism}")
    assert results is not None
