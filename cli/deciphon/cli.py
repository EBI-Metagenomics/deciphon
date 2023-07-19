from __future__ import annotations

import importlib.metadata
from pathlib import Path
from typing import Optional

from deciphon_core.hmmfile import HMMFile
from deciphon_core.press import PressContext
from deciphon_core.scan import Scan
from deciphon_core.scan_params import ScanParams
from deciphon_core.snapfile import NewSnapFile
from deciphon_snap.read_snap import read_snap
from rich.progress import track
from typer import Exit, Option, Typer, echo

from deciphon.h3daemon import H3Daemon
from deciphon.hmmer_press import hmmer_press
from deciphon.seq_file import SeqFile
from deciphon.service_exit import service_exit

__all__ = ["app"]


app = Typer(
    add_completion=False,
    pretty_exceptions_short=True,
    pretty_exceptions_show_locals=False,
)

O_PROGRESS = Option(True, "--progress/--no-progress", help="Display progress bar.")
O_LRT_THRESHOLD = Option(0.0, "--lrt-threshold", help="LRT threshold.")
O_NUM_THREADS = Option(1, "--num-threads", help="Number of threads.")
O_MULTI_HITS = Option(True, "--multi-hits/--no-multi-hits", help="Set multi-hits.")
O_HMMER3_COMPAT = Option(
    False, "--hmmer3-compat/--no-hmmer3-compat", help="Set hmmer3 compatibility."
)


@app.callback(invoke_without_command=True)
def cli(version: Optional[bool] = Option(None, "--version", is_eager=True)):
    if version:
        echo(importlib.metadata.version(__package__))
        raise Exit(0)


@app.command()
def press(hmmfile: Path, gencode: int, progress: bool = O_PROGRESS):
    """
    Make protein database.
    """
    with service_exit():
        hmm = HMMFile(path=hmmfile)
        with PressContext(hmm, gencode=gencode) as press:
            for x in track([press] * press.nproteins, "Pressing", disable=not progress):
                x.next()
            hmmer_press(hmm.path)


@app.command()
def scan(
    hmmfile: Path,
    seqfile: Path,
    snapfile: Optional[Path] = None,
    num_threads: int = O_NUM_THREADS,
    lrt_threshold: float = O_LRT_THRESHOLD,
    multi_hits: bool = O_MULTI_HITS,
    hmmer3_compat: bool = O_HMMER3_COMPAT,
    progress: bool = O_PROGRESS,
):
    """
    Scan nucleotide sequences.
    """
    with service_exit():
        hmm = HMMFile(path=hmmfile)
        snap = NewSnapFile(path=snapfile if snapfile else seqfile.with_suffix(".dcs"))

        with SeqFile(seqfile) as seq:
            with H3Daemon(hmm) as daemon:
                params = ScanParams(
                    num_threads=num_threads,
                    lrt_threshold=lrt_threshold,
                    multi_hits=multi_hits,
                    hmmer3_compat=hmmer3_compat,
                )
                scan = Scan()
                scan.dial(daemon.port)
                scan.setup(params)
                scan.run(hmm, seq, snap)


@app.command()
def see(snapfile: Path):
    """
    Display scan results.
    """
    with service_exit():
        x = read_snap(snapfile)
        echo(x)
