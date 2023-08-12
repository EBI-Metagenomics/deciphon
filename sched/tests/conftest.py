import pytest

from deciphon_sched.settings import Settings
from deciphon_sched.testing import mqtt, s3_cleanup, s3_server


@pytest.fixture
def settings():
    return Settings()


mqtt = pytest.fixture(mqtt, scope="package")
s3_server = pytest.fixture(s3_server, scope="package")


@pytest.fixture
def s3(s3_server):
    s3_cleanup(s3_server["client"])
    yield s3_server
