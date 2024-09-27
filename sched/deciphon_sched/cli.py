from functools import partial
import uvicorn
from typer import Option, Typer
from typing_extensions import Annotated

from deciphon_sched.main import create_app
from deciphon_sched.settings import Settings

app = Typer()

RELOAD = Annotated[bool, Option(help="Enable auto-reload.")]


@app.command()
def main(reload: RELOAD = False):
    settings = Settings()
    config = uvicorn.Config(
        partial(create_app, settings),
        host=settings.host,
        port=settings.port,
        log_level=settings.log_level.value,
        reload=reload,
        factory=True,
    )
    server = uvicorn.Server(config)
    server.run()
