from typing import Optional

from deciphon_poster.poster import Poster
from typer import Option, Typer
from typing_extensions import Annotated

from deciphon_worker.logging import LogLevel, setup_logger
from deciphon_worker.presser import presser_loop
from deciphon_worker.scanner import scanner_loop
from deciphon_worker.signals import raise_sigint_on_sigterm
from deciphon_worker.url import http_url

LOG_LEVEL = Annotated[LogLevel, Option(help="Log level.")]

app = Typer(
    add_completion=False,
    pretty_exceptions_short=True,
    pretty_exceptions_show_locals=False,
)


@app.command()
def scanner(
    sched_url: str,
    mqtt_host: str,
    mqtt_port: int,
    s3_url: Optional[str] = None,
    log_level: LOG_LEVEL = LogLevel.info,
):
    raise_sigint_on_sigterm()
    setup_logger(log_level)
    poster = Poster(http_url(sched_url), s3_url if s3_url is None else http_url(s3_url))
    scanner_loop(poster, mqtt_host, mqtt_port)


@app.command()
def presser(
    sched_url: str,
    mqtt_host: str,
    mqtt_port: int,
    s3_url: Optional[str] = None,
    log_level: LOG_LEVEL = LogLevel.info,
):
    raise_sigint_on_sigterm()
    setup_logger(log_level)
    poster = Poster(http_url(sched_url), s3_url if s3_url is None else http_url(s3_url))
    presser_loop(poster, mqtt_host, mqtt_port)
