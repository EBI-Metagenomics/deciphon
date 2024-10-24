from typing import Optional

import typer
from deciphon_poster.poster import Poster

from deciphon_worker.presser import presser_loop
from deciphon_worker.scanner import scanner_loop
from deciphon_worker.url import http_url

app = typer.Typer(
    add_completion=False,
    pretty_exceptions_short=True,
    pretty_exceptions_show_locals=False,
)


@app.command()
def scanner(
    sched_url: str, mqtt_host: str, mqtt_port: int, s3_url: Optional[str] = None
):
    poster = Poster(http_url(sched_url), s3_url if s3_url is None else http_url(s3_url))
    scanner_loop(poster, mqtt_host, mqtt_port)


@app.command()
def presser(
    sched_url: str, mqtt_host: str, mqtt_port: int, s3_url: Optional[str] = None
):
    poster = Poster(http_url(sched_url), s3_url if s3_url is None else http_url(s3_url))
    presser_loop(poster, mqtt_host, mqtt_port)
