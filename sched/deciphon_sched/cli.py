from enum import Enum

import uvicorn
from typer import Option, Typer
from typing_extensions import Annotated

app = Typer()


class LogLevel(str, Enum):
    critical = "critical"
    error = "error"
    warning = "warning"
    info = "info"
    debug = "debug"
    trace = "trace"

    def __str__(self):
        # Enum of Python3.10 returns a different string representation.
        # Make it return the same as in Python3.11
        return str(self.value)


HOST = Annotated[
    str,
    Option(help="Bind socket to this host."),
]
PORT = Annotated[
    int,
    Option(help="Bind socket to this port. If 0, an available port will be picked."),
]
RELOAD = Annotated[bool, Option(help="Enable auto-reload.")]
LOG_LEVEL = Annotated[LogLevel, Option(help="Log level.")]


@app.command()
def main(
    host: HOST = "0.0.0.0",
    port: PORT = 8080,
    reload: RELOAD = False,
    log_level: LOG_LEVEL = LogLevel.info,
):
    uvicorn.run(
        "deciphon_sched.main:app",
        host=host,
        port=port,
        reload=reload,
        log_level=log_level.value,
    )
