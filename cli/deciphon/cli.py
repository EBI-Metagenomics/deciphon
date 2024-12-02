from __future__ import annotations

import sys
from contextlib import suppress
from enum import Enum
from functools import partial
from pathlib import Path
from subprocess import DEVNULL
from typing import Optional
import itertools

import psutil
from deciphon_schema import DBFile, Gencode, HMMFile, NewSnapFile, SnapFile
from deciphon_snap.read_snap import read_snap
from deciphon_snap.view import view_alignments
from deciphon_worker import Progressor, launch_scanner, shutting
from deciphon_worker import press as worker
from loguru import logger
from more_itertools import mark_ends
from pydantic import ValidationError
from rich import print
from rich.progress import Progress
from typer import Argument, BadParameter, Exit, Option, Typer

from deciphon.catch_validation import catch_validation
from deciphon.hmmer_press import hmmer_press
from deciphon.read_sequences import read_sequences
from deciphon.service_exit import service_exit

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

    from rich import print

    if version:
        print(importlib.metadata.version("deciphon"))
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
    with service_exit() as srv_exit, catch_validation():
        hmm = HMMFile(path=hmmfile)

        if force and hmm.path.with_suffix(".dcp"):
            hmm.path.with_suffix(".dcp").unlink()

        def cleanup(future: Progressor[DBFile]):
            if not future.cancel():
                future.interrupt()
            with suppress(Exception):
                future.result()
            hmm.path.with_suffix(".dcp").unlink(missing_ok=True)

        future = worker(hmm, Gencode(gencode), epsilon)
        srv_exit.setup(partial(cleanup, future))

        with Progress(disable=not progress) as x:
            task = x.add_task("Pressing", total=100)
            for i in future.as_progress():
                x.update(task, completed=i)
        hmmer_press(hmm.path)

        file_dcp = hmm.path.with_suffix(".dcp")
        file_h3m = hmm.path.with_suffix(".h3m")
        file_h3i = hmm.path.with_suffix(".h3i")
        file_h3f = hmm.path.with_suffix(".h3f")
        file_h3p = hmm.path.with_suffix(".h3p")
        print(
            f"Protein database '{file_dcp}' has been successfully created\n"
            f"  alongside with HMMER files '{file_h3m}', '{file_h3i}',\n"
            f"                             '{file_h3f}', '{file_h3p}'."
        )


def find_new_snap_file(seqfile: Path):
    snap: NewSnapFile | None = None
    for i in itertools.count(start=0):
        if i == 0:
            x = seqfile.with_suffix("")
        else:
            x = seqfile.with_suffix(f".{i}")
        try:
            snap = NewSnapFile(path=x)
            break
        except ValidationError:
            continue
    assert snap is NewSnapFile


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
    logger.remove()
    logger.add(sys.stderr, filter="deciphon_worker", level="WARNING")

    with service_exit() as srv_exit, catch_validation():
        hmm = HMMFile(path=hmmfile)
        if snapfile:
            snap = NewSnapFile(path=snapfile)
        else:
            snap = find_new_snap_file(seqfile)

        if num_threads == 0:
            cpu_count = psutil.cpu_count(logical=auto_threads == AutoThreads.logical)
            if cpu_count is not None:
                num_threads = cpu_count
            else:
                num_threads = 2

        dbfile = DBFile(path=hmm.dbpath.path)
        scanner = launch_scanner(
            dbfile, num_threads, multi_hits, hmmer3_compat, False, DEVNULL, None
        ).result()
        with shutting(scanner):

            def cleanup(future: Progressor[SnapFile]):
                if not future.cancel():
                    future.interrupt()
                with suppress(Exception):
                    future.result()
                scanner.shutdown().result()
                snap.path.unlink(missing_ok=True)

            future = scanner.put(snap, read_sequences(seqfile))
            srv_exit.setup(partial(cleanup, future))

            with Progress(disable=not progress) as x:
                task = x.add_task("Scanning", total=100)
                for i in future.as_progress():
                    x.update(task, completed=i)
            print(
                "Scan has finished successfully and "
                f"results stored in '{snap.path}'."
            )


@app.command()
def see(
    snapfile: Path = Argument(..., help="File with scan results."),
):
    """
    Display scan results.
    """
    with service_exit():
        for x in mark_ends(iter(view_alignments(read_snap(snapfile)))):
            if x[1]:
                print(x[2].rstrip("\n"))
            else:
                print(x[2])


if __name__ == "__main__":
    app()
