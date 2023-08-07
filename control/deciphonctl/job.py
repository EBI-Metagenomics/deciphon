import requests

import typer
import rich
from deciphonctl.error import handle_http_error
from deciphonctl.url import url

app = typer.Typer()


@app.command()
def ls():
    response = requests.get(url("/jobs"))
    handle_http_error(response)
    rich.print(response.json())
