from __future__ import annotations

import importlib.metadata
from pathlib import Path
from subprocess import DEVNULL
from typing import Optional

from deciphon_core.params import Params
from deciphon_core.press import PressContext
from deciphon_core.scan import Scan
from deciphon_core.schema import Gencode, HMMFile, NewSnapFile
from deciphon_snap.read_snap import read_snap
from deciphon_snap.view import view_alignments
from rich.progress import track
from typer import Argument, BadParameter, Exit, Option, Typer, echo

from deciphon.catch_validation import catch_validation
from deciphon.gencode import gencodes
from deciphon.h3daemon import H3Daemon
from deciphon.hmmer_press import hmmer_press
from deciphon.progress import Progress
from deciphon.read_sequences import read_sequences
from deciphon.service_exit import service_exit

__all__ = ["app"]


app = Typer(
    add_completion=False,
    pretty_exceptions_short=True,
    pretty_exceptions_show_locals=False,
)

O_PROGRESS = Option(True, "--progress/--no-progress", help="Display progress bar.")
O_NUM_THREADS = Option(1, "--num-threads", help="Number of threads.")
O_MULTI_HITS = Option(True, "--multi-hits/--no-multi-hits", help="Set multi-hits.")
O_HMMER3_COMPAT = Option(
    False, "--hmmer3-compat/--no-hmmer3-compat", help="Set hmmer3 compatibility."
)
H_HMMER = "HMMER profile file."


@app.callback(invoke_without_command=True)
def cli(version: Optional[bool] = Option(None, "--version", is_eager=True)):
    if version:
        echo(importlib.metadata.version(__package__))
        raise Exit(0)


def gencode_callback(gencode: int):
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
    multi_hits: bool = O_MULTI_HITS,
    hmmer3_compat: bool = O_HMMER3_COMPAT,
    progress: bool = O_PROGRESS,
):
    """
    Scan nucleotide sequences against protein database.
    """
    with service_exit(), catch_validation():
        hmm = HMMFile(path=hmmfile)
        db = hmm.dbfile
        if snapfile:
            snap = NewSnapFile(path=snapfile)
        else:
            snap = NewSnapFile.create_from_prefix(seqfile.with_suffix("").name)

        sequences = read_sequences(seqfile)
        with H3Daemon(hmm, stdout=DEVNULL) as daemon:
            params = Params(num_threads, multi_hits, hmmer3_compat)
            scan = Scan(params, db)
            with scan, Progress(scan, disabled=not progress):
                scan.dial(daemon.port)
                for seq in sequences:
                    scan.add(seq)
                scan.run(snap)
        if scan.interrupted():
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
    with service_exit():
        echo(view_alignments(read_snap(snapfile)))
