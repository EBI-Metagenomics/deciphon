from __future__ import annotations

from pathlib import Path
from typing import List

import typer
from pydantic import parse_file_as
from rich import print_json

from deciphon.api.api import get_api
from deciphon.models import ScanCreate, SeqCreate

__all__ = ["app"]

app = typer.Typer(
    add_completion=False,
    pretty_exceptions_short=True,
    pretty_exceptions_show_locals=False,
)


@app.command()
def create_hmm(hmm: Path):
    get_api().create_hmm(hmm)


@app.command()
def read_hmm(hmm_id: int):
    print_json(get_api().read_hmm(hmm_id))


@app.command()
def read_hmms():
    print_json(get_api().read_hmms())


@app.command()
def create_db(db: Path):
    get_api().create_db(db)


@app.command()
def read_db(db_id: int):
    print_json(get_api().read_db(db_id))


@app.command()
def read_dbs():
    print_json(get_api().read_dbs())


@app.command()
def create_scan(db_id: int, seqs: Path):
    x = parse_file_as(List[SeqCreate], seqs, content_type="json")
    scan = ScanCreate(db_id=db_id, seqs=x, multi_hits=True, hmmer3_compat=False)
    get_api().create_scan(scan)


@app.command()
def read_scan(scan_id: int):
    print_json(get_api().read_scan(scan_id))


@app.command()
def read_scans():
    print_json(get_api().read_scans())


@app.command()
def read_job(job_id: int):
    print_json(get_api().read_job(job_id))


@app.command()
def read_jobs():
    print_json(get_api().read_jobs())


@app.command()
def create_snap(scan_id: int, snap: Path):
    get_api().create_snap(scan_id, snap)


@app.command()
def read_snap(scan_id: int):
    print_json(get_api().read_snap(scan_id))


@app.command()
def read_gff(scan_id: int):
    typer.echo(get_api().read_gff(scan_id), nl=False)


@app.command()
def read_codon(scan_id: int):
    typer.echo(get_api().read_codon(scan_id), nl=False)


@app.command()
def read_amino(scan_id: int):
    typer.echo(get_api().read_amino(scan_id), nl=False)


@app.command()
def read_state(scan_id: int):
    typer.echo(get_api().read_state(scan_id), nl=False)


@app.command()
def read_query(scan_id: int):
    typer.echo(get_api().read_query(scan_id), nl=False)


@app.command()
def read_align(scan_id: int):
    typer.echo(get_api().read_align(scan_id), nl=False)
