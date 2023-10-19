import os
from pathlib import Path

import uvicorn
import threading
import pytest
import asyncio
from deciphon_sched.main import create_app
from deciphonctl.settings import Settings
from deciphon_sched.settings import Settings as SchedSettings
from deciphon_sched.testing import mqtt_server, s3_cleanup, s3_server
from pydantic import HttpUrl
from typer.testing import CliRunner
from uvicorn.config import Config


@pytest.fixture
def settings():
    return Settings()


@pytest.fixture
def files_path() -> Path:
    current_dir = os.path.dirname(__file__)
    return Path(current_dir) / "files"


mqtt = pytest.fixture(mqtt_server, scope="package")
s3_server = pytest.fixture(s3_server, scope="package")


@pytest.fixture
def s3(s3_server):
    s3_cleanup(s3_server["client"])
    yield s3_server


class ThreadedUvicorn:
    def __init__(self, config: Config):
        self.server = uvicorn.Server(config)
        self.thread = threading.Thread(daemon=True, target=self.server.run)

    def start(self):
        self.thread.start()
        asyncio.run(self.wait_for_started())

    async def wait_for_started(self):
        while not self.server.started:
            await asyncio.sleep(0.1)

    def stop(self):
        if self.thread.is_alive():
            self.server.should_exit = True
            while self.thread.is_alive():
                continue


@pytest.fixture
def compose(mqtt, s3, settings: Settings):
    settings.mqtt_host = mqtt["host"]
    settings.mqtt_port = mqtt["port"]

    sched_settings = SchedSettings(
        mqtt_host=settings.mqtt_host,
        mqtt_port=settings.mqtt_port,
        s3_key=s3["access_key"],
        s3_secret=s3["secret_key"],
        s3_url=HttpUrl(s3["url"]),
    )

    sched = create_app(sched_settings)
    config = Config(app=sched)
    settings.sched_url = HttpUrl(f"http://{config.host}:{config.port}")
    sched_settings.allow_origins = [settings.sched_url]
    server = ThreadedUvicorn(config)

    server.start()
    yield {"sched": sched, "settings": settings}
    server.stop()


@pytest.fixture
def runner(compose):
    prefix = compose["settings"].model_config["env_prefix"]
    settings = compose["settings"]
    env = {f"{prefix}{k}": str(v) for k, v in settings.model_dump().items() if v}
    compose["runner"] = CliRunner(env={k.upper(): v for k, v in env.items()})
    yield compose["runner"]
