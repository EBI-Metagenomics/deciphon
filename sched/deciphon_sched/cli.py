import os
from functools import partial
from threading import Thread

import uvicorn
from typer import Option, Typer
from typing_extensions import Annotated

from deciphon_sched.main import create_app
from deciphon_sched.settings import Settings
from deciphon_sched.signals import raise_sigint_on_sigterm, sigint_hook

app = Typer()

RELOAD = Annotated[bool, Option(help="Enable auto-reload.")]


def wrap(server: uvicorn.Server):
    server.run()


@app.command()
def main(reload: RELOAD = False):
    raise_sigint_on_sigterm()

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

    # Send uvicorn to a subthread to prevent it from capturing signals.
    thread = Thread(target=partial(wrap, server))

    def shutdown(server: uvicorn.Server):
        if server.should_exit:
            os.abort()
            thread.start()
        server.should_exit = True

    sigint_hook(partial(shutdown, server))
    thread.start()
    thread.join()
