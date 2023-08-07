from pathlib import Path
import requests

import typer
import rich
from deciphonctl.s3 import s3_upload
from deciphonctl.error import handle_http_error
from deciphonctl.url import url

app = typer.Typer()


@app.command()
def add(db: Path):
    response = requests.get(url(f"/dbs/presigned-upload/{db.name}"))
    handle_http_error(response)

    response = s3_upload(response.json(), db)
    handle_http_error(response)

    response = requests.post(url("/dbs/"), json={"name": db.name})
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
