from __future__ import annotations

from enum import Enum
from pathlib import Path
from typing import Optional

from typer import Argument, BadParameter, Exit, Option, Typer, echo

__all__ = ["app"]


class AutoThreads(str, Enum):
    physical = "physical"
    logical = "logical"


app = Typer(
    add_completion=False,
    pretty_exceptions_short=True,
    pretty_exceptions_show_locals=False,
)

O_PROGRESS = Option(True, "--progress/--no-progress", help="Display progress bar.")
O_NUM_THREADS = Option(
    0, "--num-threads", help="Number of threads. Pass 0 for core count."
)
O_AUTO_THREADS = Option(
    AutoThreads.physical,
    "--auto-threads",
    help="Set number of threads based on core type.",
)
O_MULTI_HITS = Option(True, "--multi-hits/--no-multi-hits", help="Set multi-hits.")
O_HMMER3_COMPAT = Option(
    False, "--hmmer3-compat/--no-hmmer3-compat", help="Set hmmer3 compatibility."
)
H_HMMER = "HMMER profile file."


@app.callback(invoke_without_command=True, no_args_is_help=True)
def cli(version: Optional[bool] = Option(None, "--version", is_eager=True)):
    import importlib.metadata

    if version:
        echo(importlib.metadata.version("deciphon"))
        raise Exit(0)


def gencode_callback(gencode: int):
    from deciphon.gencode import gencodes

    if gencode not in gencodes:
        raise BadParameter(f"{gencode} is not in {gencodes}")
    return gencode


@app.command()
def press(
    hmmfile: Path = Argument(
        ..., exists=True, file_okay=True, dir_okay=False, readable=True, help=H_HMMER
    ),
    gencode: int = Argument(
        ..., callback=gencode_callback, help="Genetic code number."
    ),
    epsilon: float = Option(0.01, "--epsilon", help="Error probability."),
    progress: bool = O_PROGRESS,
    force: bool = Option(False, "--force", help="Overwrite existing protein database."),
):
    """
    Make protein database.
    """
    from deciphon_core.press import PressContext
    from deciphon_core.schema import Gencode, HMMFile
    from rich.progress import track

    from deciphon.catch_validation import catch_validation
    from deciphon.hmmer_press import hmmer_press
    from deciphon.service_exit import service_exit

    with service_exit(), catch_validation():
        hmm = HMMFile(path=hmmfile)

        if force and hmm.path.with_suffix(".dcp"):
            hmm.path.with_suffix(".dcp").unlink()

        with PressContext(hmm, gencode=Gencode(gencode), epsilon=epsilon) as press:
            for x in track([press] * press.nproteins, "Pressing", disable=not progress):
                x.next()
            hmmer_press(hmm.path)

        file_dcp = hmm.path.with_suffix(".dcp")
        file_h3m = hmm.path.with_suffix(".h3m")
        file_h3i = hmm.path.with_suffix(".h3i")
        file_h3f = hmm.path.with_suffix(".h3f")
        file_h3p = hmm.path.with_suffix(".h3p")
        echo(
            f"Protein database '{file_dcp}' has been successfully created\n"
            f"  alongside with HMMER files '{file_h3m}', '{file_h3i}',\n"
            f"                             '{file_h3f}', '{file_h3p}'."
        )


@app.command()
def scan(
    hmmfile: Path = Argument(
        ...,
        exists=True,
        file_okay=True,
        dir_okay=False,
        readable=True,
        help=H_HMMER,
    ),
    seqfile: Path = Argument(
        ...,
        exists=True,
        file_okay=True,
        dir_okay=False,
        readable=True,
        help="File with nucleotide sequences.",
    ),
    snapfile: Optional[Path] = Option(None, help="File to store results."),
    num_threads: int = O_NUM_THREADS,
    auto_threads: AutoThreads = O_AUTO_THREADS,
    multi_hits: bool = O_MULTI_HITS,
    hmmer3_compat: bool = O_HMMER3_COMPAT,
    progress: bool = O_PROGRESS,
):
    """
    Scan nucleotide sequences against protein database.
    """
    from subprocess import DEVNULL

    import psutil
    from deciphon_core.batch import Batch
    from deciphon_core.scan import Scan
    from deciphon_core.schema import HMMFile, NewSnapFile

    from deciphon.catch_validation import catch_validation
    from deciphon.h3daemon import H3Daemon
    from deciphon.progress import Progress
    from deciphon.read_sequences import read_sequences
    from deciphon.service_exit import service_exit

    with service_exit(), catch_validation():
        hmm = HMMFile(path=hmmfile)
        db = hmm.dbfile
        if snapfile:
            snap = NewSnapFile(path=snapfile)
        else:
            snap = NewSnapFile.create_from_prefix(seqfile.with_suffix("").name)

        if num_threads == 0:
            num_threads = psutil.cpu_count(logical=auto_threads == AutoThreads.logical)

        sequences = read_sequences(seqfile)
        with H3Daemon(hmm, stdout=DEVNULL) as daemon:
            scan = Scan(db, daemon.port, num_threads, multi_hits, hmmer3_compat, False)
            with scan, Progress(scan, disabled=not progress):
                batch = Batch()
                for seq in sequences:
                    batch.add(seq)
                scan.run(snap, batch)
        if scan.interrupted:
            raise Exit(1)
        snap.make_archive()
        echo("Scan has finished successfully and " f"results stored in '{snap.path}'.")


@app.command()
def see(
    snapfile: Path = Argument(..., help="File with scan results."),
):
    """
    Display scan results.
    """
    from deciphon_snap.read_snap import read_snap
    from deciphon_snap.view import view_alignments
    from more_itertools import mark_ends

    from deciphon.service_exit import service_exit

    with service_exit():
        for x in mark_ends(iter(view_alignments(read_snap(snapfile)))):
            if x[1]:
                echo(x[2].rstrip("\n"))
            else:
                echo(x[2])


if __name__ == "__main__":
    app()
