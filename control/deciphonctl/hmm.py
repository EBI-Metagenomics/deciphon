from pathlib import Path
import requests

import typer
import rich
from deciphonctl.s3 import s3_upload
from deciphonctl.error import handle_http_error
from deciphonctl.url import url


app = typer.Typer()


@app.command()
def add(hmm: Path):
    response = requests.get(url(f"/hmms/presigned-upload/{hmm.name}"))
    handle_http_error(response)

    response = s3_upload(response.json(), hmm)
    handle_http_error(response)

    response = requests.post(url("/hmms/"), json={"name": hmm.name})
    handle_http_error(response)


@app.command()
def rm(hmm_id: int):
    response = requests.delete(url(f"/hmms/{hmm_id}"))
    handle_http_error(response)


@app.command()
def ls():
    response = requests.get(url("/hmms"))
    handle_http_error(response)
    rich.print(response.json())
