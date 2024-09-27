import pytest

from deciphon_sched.logger import Logger
from deciphon_sched.settings import Settings
from deciphon_sched.testing import mqtt_server, s3_cleanup, s3_server


@pytest.fixture
def settings():
    return Settings()


@pytest.fixture
def logger(settings: Settings):
    return Logger(settings)


mqtt = pytest.fixture(mqtt_server, scope="package")
s3_server = pytest.fixture(s3_server, scope="package")


@pytest.fixture
def s3(s3_server):
    s3_cleanup(s3_server["client"])
    yield s3_server
