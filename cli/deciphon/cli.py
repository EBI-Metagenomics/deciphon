from __future__ import annotations

import importlib.metadata
import itertools
import sys
from contextlib import suppress
from enum import Enum
from functools import partial
from pathlib import Path
from typing import Annotated, Optional

import psutil
from deciphon_poster.poster import Poster
from deciphon_schema import DBFile, Gencode, HMMFile, NewSnapFile, SnapFile
from deciphon_snap.read_snap import read_snap
from deciphon_snap.view import view_alignments
from deciphon_worker import Progressor, launch_scanner
from deciphon_worker import press as launch_presser
from loguru import logger
from more_itertools import mark_ends
from pydantic import HttpUrl, TypeAdapter, ValidationError
from rich import print
from rich.progress import Progress
from typer import Argument, Exit, Option, Typer

from deciphon.catch_validation import catch_validation
from deciphon.hmmer_press import hmmer_press
from deciphon.read_sequences import read_sequences
from deciphon.service_exit import service_exit
from deciphon.worker import LogLevel, Worker, WorkType, setup_logger

__all__ = ["app"]


class AutoThreads(str, Enum):
    physical = "physical"
    logical = "logical"


app = Typer(
    add_completion=False,
    pretty_exceptions_short=True,
    pretty_exceptions_show_locals=False,
)

PROGRESS = Annotated[
    bool, Option("--progress/--no-progress", help="Display progress bar.")
]
HMMFILE = Annotated[
    Path,
    Argument(
        exists=True,
        file_okay=True,
        dir_okay=False,
        readable=True,
        help="HMMER profile file.",
        show_default=False,
    ),
]


@app.callback(invoke_without_command=True, no_args_is_help=True)
def cli(
    version: Annotated[
        Optional[bool], Option("--version", help="Show version.", is_eager=True)
    ] = None,
):
    if version:
        print(importlib.metadata.version("deciphon"))
        raise Exit(0)


class GencodeChoice(Enum):
    SGC0 = "1"
    SGC1 = "2"
    SGC2 = "3"
    SGC3 = "4"
    SGC4 = "5"
    SGC5 = "6"
    SGC8 = "9"
    SGC9 = "10"
    BAPP = "11"
    AYN = "12"
    AMC = "13"
    AFMC = "14"
    BMN = "15"
    CMC = "16"
    TMC = "21"
    SOMC = "22"
    TMMC = "23"
    PMMC = "24"
    CDSR1G = "25"
    PTN = "26"
    KN = "27"
    CN = "28"
    MN = "29"
    PN = "30"
    BN = "31"
    BP = "32"
    CMMC = "33"


@app.command()
def press(
    hmmfile: HMMFILE,
    gencode: Annotated[
        GencodeChoice,
        Argument(help="NCBI genetic code.", metavar="[GENCODE]:[1|2|...|33]"),
    ] = GencodeChoice.BAPP,
    epsilon: Annotated[float, Option("--epsilon", help="Error probability.")] = 0.01,
    progress: PROGRESS = True,
    force: Annotated[
        bool, Option("--force", help="Overwrite existing protein database.")
    ] = False,
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

        future = launch_presser(hmm, Gencode(int(gencode.value)), epsilon)
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
            x = seqfile.with_suffix(".dcs")
        else:
            x = seqfile.with_suffix(f".{i}.dcs")
        try:
            snap = NewSnapFile(path=x)
            break
        except ValidationError:
            continue
    assert snap is not None
    return snap


@app.command()
def scan(
    hmmfile: HMMFILE,
    seqfile: Annotated[
        Path,
        Argument(
            exists=True,
            file_okay=True,
            dir_okay=False,
            readable=True,
            help="File with nucleotide sequences.",
            show_default=False,
        ),
    ],
    snapfile: Annotated[
        Optional[Path], Option(help="File to store results.", show_default=False)
    ] = None,
    num_threads: Annotated[
        int, Option("--num-threads", help="Number of threads. Set 0 for core count.")
    ] = 0,
    auto_threads: Annotated[
        AutoThreads,
        Option("--auto-threads", help="Set number of threads based on core type."),
    ] = AutoThreads.physical,
    multi_hits: Annotated[
        bool, Option("--multi-hits/--no-multi-hits", help="Set multi-hits.")
    ] = True,
    hmmer3_compat: Annotated[
        bool,
        Option("--hmmer3-compat/--no-hmmer3-compat", help="Set HMMER3 compatibility."),
    ] = False,
    progress: PROGRESS = True,
):
    """
    Scan nucleotide sequence against protein database.
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
            dbfile, num_threads, multi_hits, hmmer3_compat, False, None, None
        ).result()
        with scanner:

            def cleanup(future: Progressor[SnapFile]):
                if not future.cancel():
                    future.interrupt()
                with suppress(Exception):
                    future.result()
                snap.path.unlink(missing_ok=True)

            future = scanner.put(snap, read_sequences(seqfile))
            srv_exit.setup(partial(cleanup, future))

            with Progress(disable=not progress) as x:
                task = x.add_task("Scanning", total=100)
                for i in future.as_progress():
                    x.update(task, completed=i)
            print(
                f"Scan has finished successfully and results stored in '{snap.path}'."
            )


@app.command()
def see(
    snapfile: Annotated[
        Path, Argument(help="File with scan results.", show_default=False)
    ],
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


def http_url(url: str) -> HttpUrl:
    return TypeAdapter(HttpUrl).validate_strings(url)


class shutdown_worker:
    def __init__(self, worker: Worker):
        self.worker = worker
        self.count = 0

    def __call__(self):
        immediate = self.count > 0
        self.count += 1
        self.worker.shutdown(immediate=immediate)


@app.command()
def worker(
    work: Annotated[WorkType, Argument(help="Work type.", show_default=False)],
    sched_url: Annotated[str, Argument(help="Scheduler URL.", show_default=False)],
    mqtt_host: Annotated[str, Argument(help="MQTT host.", show_default=False)],
    mqtt_port: Annotated[int, Option(help="MQTT port.")] = 1883,
    s3_url: Annotated[Optional[str], Option(help="S3 url.", show_default=False)] = None,
    log_level: Annotated[LogLevel, Option(help="Log level.")] = LogLevel.info,
):
    """
    Launch worker.
    """
    with service_exit() as srv_exit, catch_validation():
        setup_logger(log_level)
        poster = Poster(
            http_url(sched_url), s3_url if s3_url is None else http_url(s3_url)
        )
        w = Worker(work, poster, mqtt_host, mqtt_port)
        srv_exit.setup(shutdown_worker(w))
        w.loop_forever()


if __name__ == "__main__":
    app()
