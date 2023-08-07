import requests

from typing_extensions import Annotated
import typer
import rich
from deciphonctl.error import handle_http_error
from deciphonctl.url import url
import fasta_reader
from pydantic import BaseModel


app = typer.Typer()


class Seq(BaseModel):
    name: str
    data: str


class Scan(BaseModel):
    db_id: int
    multi_hits: bool
    hmmer3_compat: bool
    seqs: list[Seq]


@app.command()
def add(
    fasta: Annotated[typer.FileText, typer.Argument()],
    db_id: int,
    multi_hits: bool = True,
    hmmer3_compat: bool = False,
):
    seqs = [Seq(name=x.id, data=x.sequence) for x in fasta_reader.Reader(fasta)]
    x = Scan(db_id=db_id, multi_hits=multi_hits, hmmer3_compat=hmmer3_compat, seqs=seqs)
    response = requests.post(url("/scans/"), json=x.model_dump())
    handle_http_error(response)


@app.command()
def rm(scan_id: int):
    response = requests.delete(url(f"/scans/{scan_id}"))
    handle_http_error(response)


@app.command()
def ls():
    response = requests.get(url("/scans"))
    handle_http_error(response)
    rich.print(response.json())
