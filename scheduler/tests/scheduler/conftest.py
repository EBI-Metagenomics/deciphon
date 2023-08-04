import pytest
from pydantic import AnyHttpUrl

from deciphon_scheduler.main import create_app
from deciphon_scheduler.settings import Settings


@pytest.fixture
def app(mqtt, s3, settings: Settings):
    settings.mqtt_host = mqtt["host"]
    settings.mqtt_port = mqtt["port"]
    settings.s3_key = s3["access_key"]
    settings.s3_secret = s3["secret_key"]
    settings.s3_url = AnyHttpUrl(s3["url"])
    yield create_app(settings)
