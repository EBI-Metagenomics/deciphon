import sys
from pathlib import Path
from pprint import pformat

import fasta_reader
import requests
import rich
import typer
from deciphon_core.schema import DBName, Gencode, HMMName
from deciphon_poster.errors import PosterHTTPError
from deciphon_poster.poster import Poster
from deciphon_poster.schema import Scan, Seq
from typer import Argument, FileText, Option, echo
from typing_extensions import Annotated

from deciphonctl.catch_validation import catch_validation
from deciphonctl.display_exception import display_exception
from deciphonctl.log_level import LogLevel
from deciphonctl.progress import Progress
from deciphonctl.settings import (
    Settings,
    SettingsFields,
    cfg_file,
    cfg_file_set,
    cfg_vars,
    env_vars,
)

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
CFGOPT = Annotated[SettingsFields, Argument(help="Config option")]
CFGVAL = Annotated[str, Argument(help="Config value")]

EXCEPTIONS_FOR_DISPLAY = (
    ConnectionError,
    PosterHTTPError,
    requests.exceptions.ConnectionError,
)

config = typer.Typer()
hmm = typer.Typer()
db = typer.Typer()
job = typer.Typer()
scan = typer.Typer()
seq = typer.Typer()
snap = typer.Typer()
presser = typer.Typer()
scanner = typer.Typer()


@config.command("debug")
@catch_validation
@display_exception(EXCEPTIONS_FOR_DISPLAY)
def config_debug():
    echo("configuration file:")
    echo(f"  {cfg_file()}, exists: {cfg_file().exists()}")
    if cfg_file().exists():
        for field, value in cfg_vars().items():
            echo(f"    {field}={pformat(value)}")

    echo("environment variables:")
    for field, value in env_vars().items():
        echo(f"  {field}={pformat(value)}")

    echo("final configuration:")
    settings = Settings()
    for field in settings.__fields__:
        value = getattr(settings, field)
        if value is None:
            echo(f"  {field}=")
        else:
            echo(f"  {field}={pformat(value)}")


@config.command("set")
@catch_validation
@display_exception(EXCEPTIONS_FOR_DISPLAY)
def config_set(option: CFGOPT, value: CFGVAL):
    field = option.value
    Settings.model_validate({field: value})
    echo(f"Writing to {cfg_file()}")
    cfg_file_set(cfg_file(), field, value)


@config.command("get")
@catch_validation
@display_exception(EXCEPTIONS_FOR_DISPLAY)
def config_get(option: CFGOPT):
    field = option.value
    settings = Settings()
    echo(settings.model_dump()[field])


@hmm.command("add")
@catch_validation
@display_exception(EXCEPTIONS_FOR_DISPLAY)
def hmm_add(hmmfile: HMMFILE, gencode: GENCODE, epsilon: EPSILON = 0.01):
    settings = Settings()
    poster = Poster(settings.sched_url, settings.s3_url)
    upload_post = poster.upload_hmm_post(hmmfile.name)
    with Progress() as progress:
        poster.upload(hmmfile, upload_post, progress)
    poster.hmm_post(HMMName(name=hmmfile.name), gencode, epsilon)


@hmm.command("rm")
@catch_validation
@display_exception(EXCEPTIONS_FOR_DISPLAY)
def hmm_rm(hmm_id: HMMID):
    settings = Settings()
    poster = Poster(settings.sched_url, settings.s3_url)
    poster.hmm_delete(hmm_id)


@hmm.command("ls")
@catch_validation
@display_exception(EXCEPTIONS_FOR_DISPLAY)
def hmm_ls():
    settings = Settings()
    poster = Poster(settings.sched_url, settings.s3_url)
    rich.print(poster.hmm_list())


@db.command("add")
@catch_validation
@display_exception(EXCEPTIONS_FOR_DISPLAY)
def db_add(dbfile: DBFILE, gencode: GENCODE, epsilon: EPSILON = 0.01):
    settings = Settings()
    poster = Poster(settings.sched_url, settings.s3_url)
    upload_post = poster.upload_db_post(dbfile.name)
    with Progress() as progress:
        poster.upload(dbfile, upload_post, progress)
    poster.db_post(DBName(name=dbfile.name))


@db.command("rm")
@catch_validation
@display_exception(EXCEPTIONS_FOR_DISPLAY)
def db_rm(db_id: DBID):
    settings = Settings()
    poster = Poster(settings.sched_url, settings.s3_url)
    poster.db_delete(db_id)


@db.command("ls")
@catch_validation
@display_exception(EXCEPTIONS_FOR_DISPLAY)
def db_ls():
    settings = Settings()
    poster = Poster(settings.sched_url, settings.s3_url)
    rich.print(poster.db_list())


@job.command("ls")
@catch_validation
@display_exception(EXCEPTIONS_FOR_DISPLAY)
def job_ls():
    settings = Settings()
    poster = Poster(settings.sched_url, settings.s3_url)
    rich.print(poster.job_list())


@scan.command("add")
@catch_validation
@display_exception(EXCEPTIONS_FOR_DISPLAY)
def scan_add(
    fasta: FASTAFILE,
    db_id: DBID,
    multi_hits: MULTIHITS = True,
    hmmer3_compat: HMMER3COMPAT = False,
):
    settings = Settings()
    seqs = [Seq(name=x.id, data=x.sequence) for x in fasta_reader.Reader(fasta)]
    x = Scan(db_id=db_id, multi_hits=multi_hits, hmmer3_compat=hmmer3_compat, seqs=seqs)
    poster = Poster(settings.sched_url, settings.s3_url)
    poster.scan_post(x)


@scan.command("rm")
@catch_validation
@display_exception(EXCEPTIONS_FOR_DISPLAY)
def scan_rm(scan_id: SCANID):
    settings = Settings()
    poster = Poster(settings.sched_url, settings.s3_url)
    poster.scan_delete(scan_id)


@scan.command("ls")
@catch_validation
@display_exception(EXCEPTIONS_FOR_DISPLAY)
def scan_ls():
    settings = Settings()
    poster = Poster(settings.sched_url, settings.s3_url)
    rich.print(poster.scan_list())


@seq.command("ls")
@catch_validation
@display_exception(EXCEPTIONS_FOR_DISPLAY)
def seq_ls():
    settings = Settings()
    poster = Poster(settings.sched_url, settings.s3_url)
    rich.print(poster.seq_list())


@scan.command("snap-add")
@catch_validation
@display_exception(EXCEPTIONS_FOR_DISPLAY)
def snap_add(scan_id: SCANID, snap: SNAPFILE):
    settings = Settings()
    poster = Poster(settings.sched_url, settings.s3_url)
    poster.snap_post(scan_id, snap)


@scan.command("snap-get")
@catch_validation
@display_exception(EXCEPTIONS_FOR_DISPLAY)
def snap_get(scan_id: SCANID, output_file: OUTFILE = Path("snap.dcs")):
    settings = Settings()
    poster = Poster(settings.sched_url, settings.s3_url)
    with open(output_file, "wb") as file:
        file.write(poster.snap_get(scan_id))


@scan.command("snap-rm")
@catch_validation
@display_exception(EXCEPTIONS_FOR_DISPLAY)
def snap_rm(scan_id: SCANID):
    settings = Settings()
    poster = Poster(settings.sched_url, settings.s3_url)
    poster.snap_delete(scan_id)


@scan.command("snap-view")
@catch_validation
@display_exception(EXCEPTIONS_FOR_DISPLAY)
def snap_view(scan_id: SCANID):
    settings = Settings()
    poster = Poster(settings.sched_url, settings.s3_url)
    print(poster.snap_view(scan_id))


LOG_LEVEL = Annotated[LogLevel, Option(help="Log level.")]


app = typer.Typer(
    add_completion=False,
    pretty_exceptions_short=True,
    pretty_exceptions_show_locals=False,
)
app.add_typer(config, name="config")
app.add_typer(hmm, name="hmm")
app.add_typer(db, name="db")
app.add_typer(job, name="job")
app.add_typer(scan, name="scan")
app.add_typer(seq, name="seq")
app.add_typer(presser, name="presser")
app.add_typer(scanner, name="scanner")

if __name__ == "__main__":
    sys.exit(app())
