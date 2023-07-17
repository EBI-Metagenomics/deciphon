import pickle as pkl
from pathlib import Path

import fire
import polars as pl
from profile_index import ProfileIndex
from qual_index import QualIndex
from sequence_index import SequenceIndex

from deciphon_eval.confusion import ConfusionMatrix

schema = {
    "precision": pl.Float64,
    "recall": pl.Float64,
    "domain": pl.Utf8,
    "organism": pl.Utf8,
    "P": pl.Int64,
    "N": pl.Int64,
    "TP": pl.Int64,
    "FP": pl.Int64,
}


def score_qualitatively(parquetfile: str, dbfile: str, *genome_dirs):
    parquet = Path(parquetfile)
    if parquet.suffix != ".parquet":
        raise ValueError("Output file must end with .parquet.")

    data = {k: [] for k in schema.keys()}

    for x in genome_dirs:
        genome_dir = Path(x)
        fasta = Path(genome_dir) / "cds_from_genomic.fna"
        index = QualIndex(SequenceIndex(str(fasta)), ProfileIndex(dbfile))

        guessed_file = genome_dir / "guessed_positives_qual.pkl"
        positives_file = genome_dir / "positives_qual.pkl"

        if not guessed_file.exists():
            print(f"File <{guessed_file}> does not exist. Skipping genome.")
            continue

        if not positives_file.exists():
            print(f"File <{positives_file}> does not exist. Skipping genome.")
            continue

        with open(guessed_file, "rb") as f:
            guessed_positives = pkl.load(f)

        with open(positives_file, "rb") as f:
            positives = pkl.load(f)

        P = len(positives)
        N = index.count - P

        true_positives = positives & guessed_positives
        TP = len(true_positives)
        false_positives = guessed_positives - true_positives
        FP = len(false_positives)
        cm = ConfusionMatrix(P=P, N=N, TP=TP, FP=FP)
        print(f"Precision={cm.precision},Recall={cm.recall} {genome_dir}")

        data["precision"].append(cm.precision)
        data["recall"].append(cm.recall)
        data["domain"].append(genome_dir.parent.name)
        data["organism"].append(genome_dir.name)
        data["P"].append(cm.P)
        data["N"].append(cm.N)
        data["TP"].append(cm.TP)
        data["FP"].append(cm.FP)

    df = pl.DataFrame(data, schema=schema)
    df.write_parquet(parquet)


if __name__ == "__main__":
    fire.Fire(score_qualitatively)
