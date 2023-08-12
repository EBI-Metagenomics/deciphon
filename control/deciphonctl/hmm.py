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

HMMFILE = Annotated[Path, typer.Argument(help="Path to an HMM file")]


@app.command()
def add(hmmfile: HMMFILE, gencode: GENCODE, epsilon: EPSILON = 0.01):
    response = requests.get(url(f"/hmms/presigned-upload/{hmmfile.name}"))
    handle_http_error(response)

    response = s3_upload(response.json(), hmmfile)
    handle_http_error(response)

    response = requests.post(
        url("/hmms/"),
        params={"gencode": int(gencode), "epsilon": epsilon},
        json={"name": hmmfile.name},
    )
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
