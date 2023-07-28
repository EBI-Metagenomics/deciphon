import pytest
from fastapi.testclient import TestClient

from deciphon_scheduler.main import create_app
from deciphon_scheduler.settings import Settings

MINIFAM_HMM = {
    "file": {
        "name": "minifam.hmm",
        "sha256": "fe305d9c09e123f987f49b9056e34c374e085d8831f815cc73d8ea4cdec84960",
    }
}


class Url:
    def __init__(self, settings: Settings):
        self._prefix = settings.endpoint_prefix

    def __call__(self, endpoint: str):
        return self._prefix + endpoint


@pytest.fixture(scope="module")
def settings():
    return Settings()


@pytest.fixture
def url(settings):
    return Url(settings)


@pytest.fixture
def app(mosquitto, settings):
    return create_app(settings)


def test_hmm_not_found(app, url):
    with TestClient(app) as client:
        response = client.get(url("/hmms/1"))
        assert response.status_code == 404


def test_create_hmm(app, url):
    with TestClient(app) as client:
        response = client.post(url("/hmms/"), json=MINIFAM_HMM)
        assert response.status_code == 201


def test_read_hmm(app, url):
    with TestClient(app) as client:
        response = client.post(url("/hmms/"), json=MINIFAM_HMM)
        assert response.status_code == 201
        response = client.get(url("/hmms/1"))
        assert response.status_code == 200


def test_read_hmms(app, url):
    with TestClient(app) as client:
        response = client.post(url("/hmms/"), json=MINIFAM_HMM)
        assert response.status_code == 201
        response = client.get(url("/hmms"))
        assert response.status_code == 200


def test_delete_hmm(app, url):
    with TestClient(app) as client:
        response = client.post(url("/hmms/"), json=MINIFAM_HMM)
        assert response.status_code == 201
        response = client.delete(url("/hmms/1"))
        assert response.status_code == 204
