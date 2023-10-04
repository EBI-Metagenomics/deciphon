import pickle as pkl
from pathlib import Path

import fire
import polars as pl
from profile_index import ProfileIndex
from profile_nucleotide_index import ProfileNucleotideIndex
from read_nucltdb import compute_genome_size, read_nucltdb

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


def score_quantitatively(output_file: str, pfam_file: str, *genome_dirs):
    output = Path(output_file)
    if output.suffix != ".parquet":
        raise ValueError("Output file must end with .parquet.")

    data = {k: [] for k in schema.keys()}

    print(f"Processing {len(genome_dirs)} genomes.")
    for x in genome_dirs:
        genome_dir = Path(x)
        fasta = Path(genome_dir) / "cds_from_genomic.fna"

        nucltdb = read_nucltdb(fasta)
        profile_index = ProfileIndex(pfam_file)
        pair = ProfileNucleotideIndex(profile_index, compute_genome_size(nucltdb))

        guessed_file = genome_dir / "guessed_positive_nucleotides.pkl"
        positives_file = genome_dir / "true_positive_nucleotides.pkl"

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
        N = pair.count - P

        true_positives = positives & guessed_positives
        TP = sum(interval_size(x) for x in true_positives)
        false_positives = guessed_positives - true_positives
        FP = sum(interval_size(x) for x in false_positives)
        cm = ConfusionMatrix(P=P, N=N, TP=TP, FP=FP)
        print(f"Precision={cm.precision:.2g},Recall={cm.recall:.2g} {genome_dir}")

        data["precision"].append(cm.precision)
        data["recall"].append(cm.recall)
        data["domain"].append(genome_dir.parent.name)
        data["organism"].append(genome_dir.name)
        data["P"].append(cm.P)
        data["N"].append(cm.N)
        data["TP"].append(cm.TP)
        data["FP"].append(cm.FP)

    print(f"Writting {output}...", end=" ", flush=True)
    df = pl.DataFrame(data, schema=schema)
    df.write_parquet(output)
    print("done.")


if __name__ == "__main__":
    fire.Fire(score_quantitatively)
