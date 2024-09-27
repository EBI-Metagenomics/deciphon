import pathlib

import pytest
import requests
from fastapi.testclient import TestClient

from deciphon_sched.main import create_app
from deciphon_sched.settings import Settings


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
    data = settings.model_dump()
    data["mqtt_host"] = mqtt["host"]
    data["mqtt_port"] = mqtt["port"]
    data["s3_key"] = s3["access_key"]
    data["s3_secret"] = s3["secret_key"]
    data["s3_url"] = s3["url"]
    settings = Settings.model_validate(data)
    yield create_app(settings)


@pytest.fixture
def client(compose):
    with TestClient(compose) as client:
        yield client


@pytest.fixture()
def files() -> pathlib.Path:
    return pathlib.Path(__file__).parent / "files"
