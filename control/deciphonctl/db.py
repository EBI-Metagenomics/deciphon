from pathlib import Path

import requests
import rich
import typer
from typing_extensions import Annotated

from deciphonctl.error import handle_http_error
from deciphonctl.s3 import s3_upload
from deciphonctl.schemas import EPSILON, GENCODE
from deciphonctl.url import url

app = typer.Typer()

DBFILE = Annotated[Path, typer.Argument(help="Path to an DB file")]


@app.command()
def add(dbfile: DBFILE, gencode: GENCODE, epsilon: EPSILON = 0.01):
    response = requests.get(url(f"/dbs/presigned-upload/{dbfile.name}"))
    handle_http_error(response)

    response = s3_upload(response.json(), dbfile)
    handle_http_error(response)

    response = requests.post(
        url("/dbs/"),
        json={"name": dbfile.name, "gencode": int(gencode), "epsilon": epsilon},
    )
    handle_http_error(response)


@app.command()
def rm(db_id: int):
    response = requests.delete(url(f"/dbs/{db_id}"))
    handle_http_error(response)


@app.command()
def ls():
    response = requests.get(url("/dbs"))
    handle_http_error(response)
    rich.print(response.json())
