import pytest
import requests
from fastapi.testclient import TestClient
from pydantic import AnyHttpUrl

import pathlib
from deciphon_scheduler.main import create_app
from deciphon_scheduler.settings import Settings


@pytest.fixture
def s3_upload():
    def upload(presigned_upload, file):
        with open(file, "rb") as f:
            files = {"file": (file.name, f)}
            url = presigned_upload["url"]
            fields = presigned_upload["fields"]
            http_response = requests.post(url, data=fields, files=files)
            assert http_response.status_code == 204

    yield upload


@pytest.fixture
def compose(mqtt, s3, settings: Settings):
    settings.mqtt_host = mqtt["host"]
    settings.mqtt_port = mqtt["port"]
    settings.s3_key = s3["access_key"]
    settings.s3_secret = s3["secret_key"]
    settings.s3_url = AnyHttpUrl(s3["url"])
    yield {"app": create_app(settings), "s3": s3["container"], "settings": settings}


@pytest.fixture
def client(compose):
    with TestClient(compose["app"]) as client:
        yield client


@pytest.fixture()
def files() -> pathlib.Path:
    return pathlib.Path(__file__).parent / "files"
