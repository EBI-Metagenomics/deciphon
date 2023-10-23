import uvicorn
from typer import Option, Typer
from typing_extensions import Annotated

from deciphon_sched.settings import Settings

app = Typer()

RELOAD = Annotated[bool, Option(help="Enable auto-reload.")]

@app.command()
def main(reload: RELOAD = False):
    settings = Settings()
    uvicorn.run(
        "deciphon_sched.main:app",
        host=settings.host,
        port=settings.port,
        reload=reload,
        log_level=settings.log_level.value,
    )
