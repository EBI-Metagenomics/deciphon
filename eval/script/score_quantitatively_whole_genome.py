import pickle as pkl
from pathlib import Path

import fire
import polars as pl
from profile_index import ProfileIndex
from read_nucltdb import compute_genome_size, read_nucltdb
from whole_genome_index import WholeGenomeIndex

from deciphon_eval.confusion import ConfusionMatrix
from deciphon_eval.portion_utils import interval_size

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


def score_quantitatively_whole_genome(output: str, dbfile: str, *genome_dirs):
    parquet = Path(output)
    if parquet.suffix != ".parquet":
        raise ValueError("Output file must end with .parquet.")

    data = {k: [] for k in schema.keys()}

    for x in genome_dirs:
        genome_dir = Path(x)
        nucltdb = read_nucltdb(genome_dir / "cds_from_genomic.fna")
        index = WholeGenomeIndex(ProfileIndex(dbfile), compute_genome_size(nucltdb))

        guessed_file = genome_dir / "guessed_positives.pkl"
        positives_file = genome_dir / "positives.pkl"

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

        P = sum(interval_size(x) for x in positives)
        N = index.count - P

        true_positives = positives & guessed_positives
        TP = sum(interval_size(x) for x in true_positives)
        false_positives = guessed_positives - true_positives
        FP = sum(interval_size(x) for x in false_positives)
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
    fire.Fire(score_quantitatively_whole_genome)
