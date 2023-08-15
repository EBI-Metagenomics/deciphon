from pathlib import Path

import fasta_reader
import rich
import typer
from deciphon_core.schema import Gencode
from typer import Argument, FileText
from typing_extensions import Annotated

from deciphonctl import settings
from deciphonctl.models import DBFile, HMMFile, Scan, Seq
from deciphonctl.presser import presser_entry
from deciphonctl.sched import Sched

HMMFILE = Annotated[Path, Argument(help="Path to an HMM file")]
DBFILE = Annotated[Path, Argument(help="Path to an DB file")]
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

hmm = typer.Typer()
db = typer.Typer()
job = typer.Typer()
scan = typer.Typer()
presser = typer.Typer()


@hmm.command("add")
def hmm_add(hmmfile: HMMFILE, gencode: GENCODE, epsilon: EPSILON = 0.01):
    sched = Sched(settings.sched_url)
    sched.upload(hmmfile, sched.presigned.upload_hmm_post(hmmfile.name))
    sched.hmm_post(HMMFile(name=hmmfile.name), gencode, epsilon)


@hmm.command("rm")
def hmm_rm(hmm_id: HMMID):
    sched = Sched(settings.sched_url)
    sched.hmm_delete(hmm_id)


@hmm.command("ls")
def hmm_ls():
    sched = Sched(settings.sched_url)
    rich.print(sched.hmm_list())


@db.command("add")
def db_add(dbfile: DBFILE, gencode: GENCODE, epsilon: EPSILON = 0.01):
    sched = Sched(settings.sched_url)
    sched.upload(dbfile, sched.presigned.upload_db_post(dbfile.name))
    sched.db_post(DBFile(name=dbfile.name, gencode=gencode, epsilon=epsilon))


@db.command("rm")
def db_rm(db_id: DBID):
    sched = Sched(settings.sched_url)
    sched.db_delete(db_id)


@db.command("ls")
def db_ls():
    sched = Sched(settings.sched_url)
    rich.print(sched.db_list())


@job.command("ls")
def job_ls():
    sched = Sched(settings.sched_url)
    rich.print(sched.job_list())


@scan.command("add")
def scan_add(
    fasta: FASTAFILE,
    db_id: DBID,
    multi_hits: MULTIHITS = True,
    hmmer3_compat: HMMER3COMPAT = False,
):
    seqs = [Seq(name=x.id, data=x.sequence) for x in fasta_reader.Reader(fasta)]
    x = Scan(db_id=db_id, multi_hits=multi_hits, hmmer3_compat=hmmer3_compat, seqs=seqs)
    sched = Sched(settings.sched_url)
    sched.scan_post(x)


@scan.command("rm")
def scan_rm(scan_id: SCANID):
    sched = Sched(settings.sched_url)
    sched.scan_delete(scan_id)


@scan.command("ls")
def scan_ls():
    sched = Sched(settings.sched_url)
    rich.print(sched.scan_list())


@presser.command("start")
def presser_start(num_workers: int = 1):
    sched = Sched(settings.sched_url)
    presser_entry(sched, num_workers)


app = typer.Typer()
app.add_typer(hmm, name="hmm")
app.add_typer(db, name="db")
app.add_typer(job, name="job")
app.add_typer(scan, name="scan")
app.add_typer(presser, name="presser")
