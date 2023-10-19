import sys
from pathlib import Path

import fasta_reader
import rich
import typer
from deciphon_core.schema import Gencode
from loguru import logger
from typer import Argument, FileText, Option
from typing_extensions import Annotated

from deciphonctl.settings import Settings
from deciphonctl.models import DBFile, HMMFile, LogLevel, Scan, Seq
from deciphonctl.presser import presser_entry
from deciphonctl.scanner import scanner_entry
from deciphonctl.sched import Sched
from deciphonctl.signals import raise_sigint_on_sigterm

HMMFILE = Annotated[
    Path,
    Argument(
        exists=True,
        file_okay=True,
        dir_okay=False,
        readable=True,
        help="Path to an HMM file",
    ),
]
DBFILE = Annotated[
    Path,
    Argument(
        exists=True,
        file_okay=True,
        dir_okay=False,
        readable=True,
        help="Path to an DB file",
    ),
]
GENCODE = Annotated[
    Gencode, Argument(parser=lambda x: Gencode(int(x)), help="NCBI genetic code number")
]
HMMID = Annotated[int, Argument(help="HMM ID")]
DBID = Annotated[int, Argument(help="Database ID")]
SCANID = Annotated[int, Argument(help="Scan ID")]
EPSILON = Annotated[float, Argument(help="Nucleotide error probability")]
FASTAFILE = Annotated[FileText, Argument(help="FASTA file")]
MULTIHITS = Annotated[bool, Argument(help="Enable multiple-hits")]
HMMER3COMPAT = Annotated[bool, Argument(help="Enable HMMER3 compatibility")]
SNAPFILE = Annotated[
    Path,
    Argument(
        exists=True, file_okay=True, dir_okay=False, readable=True, help="Snap file"
    ),
]
OUTFILE = Annotated[
    Path,
    Option(file_okay=True, dir_okay=False, writable=True, help="Output file"),
]

hmm = typer.Typer()
db = typer.Typer()
job = typer.Typer()
scan = typer.Typer()
seq = typer.Typer()
snap = typer.Typer()
presser = typer.Typer()
scanner = typer.Typer()


@hmm.command("add")
def hmm_add(hmmfile: HMMFILE, gencode: GENCODE, epsilon: EPSILON = 0.01):
    settings = Settings()
    sched = Sched(settings.sched_url, settings.s3_url)
    sched.upload(hmmfile, sched.presigned.upload_hmm_post(hmmfile.name))
    sched.hmm_post(HMMFile(name=hmmfile.name), gencode, epsilon)


@hmm.command("rm")
def hmm_rm(hmm_id: HMMID):
    settings = Settings()
    sched = Sched(settings.sched_url, settings.s3_url)
    sched.hmm_delete(hmm_id)


@hmm.command("ls")
def hmm_ls():
    settings = Settings()
    sched = Sched(settings.sched_url, settings.s3_url)
    rich.print(sched.hmm_list())


@db.command("add")
def db_add(dbfile: DBFILE, gencode: GENCODE, epsilon: EPSILON = 0.01):
    settings = Settings()
    sched = Sched(settings.sched_url, settings.s3_url)
    sched.upload(dbfile, sched.presigned.upload_db_post(dbfile.name))
    sched.db_post(DBFile(name=dbfile.name, gencode=gencode, epsilon=epsilon))


@db.command("rm")
def db_rm(db_id: DBID):
    settings = Settings()
    sched = Sched(settings.sched_url, settings.s3_url)
    sched.db_delete(db_id)


@db.command("ls")
def db_ls():
    settings = Settings()
    sched = Sched(settings.sched_url, settings.s3_url)
    rich.print(sched.db_list())


@job.command("ls")
def job_ls():
    settings = Settings()
    sched = Sched(settings.sched_url, settings.s3_url)
    rich.print(sched.job_list())


@scan.command("add")
def scan_add(
    fasta: FASTAFILE,
    db_id: DBID,
    multi_hits: MULTIHITS = True,
    hmmer3_compat: HMMER3COMPAT = False,
):
    settings = Settings()
    seqs = [Seq(name=x.id, data=x.sequence) for x in fasta_reader.Reader(fasta)]
    x = Scan(db_id=db_id, multi_hits=multi_hits, hmmer3_compat=hmmer3_compat, seqs=seqs)
    sched = Sched(settings.sched_url, settings.s3_url)
    sched.scan_post(x)


@scan.command("rm")
def scan_rm(scan_id: SCANID):
    settings = Settings()
    sched = Sched(settings.sched_url, settings.s3_url)
    sched.scan_delete(scan_id)


@scan.command("ls")
def scan_ls():
    settings = Settings()
    sched = Sched(settings.sched_url, settings.s3_url)
    rich.print(sched.scan_list())


@seq.command("ls")
def seq_ls():
    settings = Settings()
    sched = Sched(settings.sched_url, settings.s3_url)
    rich.print(sched.seq_list())


@scan.command("snap-add")
def snap_add(scan_id: SCANID, snap: SNAPFILE):
    settings = Settings()
    sched = Sched(settings.sched_url, settings.s3_url)
    sched.snap_post(scan_id, snap)


@scan.command("snap-get")
def snap_get(scan_id: SCANID, output_file: OUTFILE = Path("snap.dcs")):
    settings = Settings()
    sched = Sched(settings.sched_url, settings.s3_url)
    with open(output_file, "wb") as file:
        file.write(sched.snap_get(scan_id))


@scan.command("snap-rm")
def snap_rm(scan_id: SCANID):
    settings = Settings()
    sched = Sched(settings.sched_url, settings.s3_url)
    sched.snap_delete(scan_id)


@scan.command("snap-view")
def snap_view(scan_id: SCANID):
    settings = Settings()
    sched = Sched(settings.sched_url, settings.s3_url)
    print(sched.snap_view(scan_id))


LOG_LEVEL = Annotated[LogLevel, Option(help="Log level.")]


@presser.command("run")
def presser_run(num_workers: int = 1, log_level: LOG_LEVEL = LogLevel.info):
    settings = Settings()
    raise_sigint_on_sigterm()
    logger.remove()
    logger.add(sys.stderr, level=log_level.value.upper())
    sched = Sched(settings.sched_url, settings.s3_url)
    presser_entry(settings, sched, num_workers)


@scanner.command("run")
def scanner_run(
    num_workers: int = 1, num_threads: int = 1, log_level: LOG_LEVEL = LogLevel.info
):
    settings = Settings()
    raise_sigint_on_sigterm()
    logger.remove()
    logger.add(sys.stderr, level=log_level.value.upper())
    sched = Sched(settings.sched_url, settings.s3_url)
    scanner_entry(settings, sched, num_workers, num_threads)


app = typer.Typer()
app.add_typer(hmm, name="hmm")
app.add_typer(db, name="db")
app.add_typer(job, name="job")
app.add_typer(scan, name="scan")
app.add_typer(seq, name="seq")
app.add_typer(presser, name="presser")
app.add_typer(scanner, name="scanner")

if __name__ == "__main__":
    sys.exit(app())
