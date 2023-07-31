import pytest
from fastapi.testclient import TestClient

from deciphon_scheduler.main import create_app
from deciphon_scheduler.settings import Settings

MINIFAM_HMM = {"file": {"name": "minifam.hmm"}}


@pytest.fixture(scope="module")
def settings():
    return Settings()


@pytest.fixture
def app(mosquitto, settings):
    return create_app(settings)


def test_hmm_not_found(app):
    with TestClient(app) as client:
        response = client.get("/hmms/1")
        assert response.status_code == 404


def test_create_hmm(app):
    with TestClient(app) as client:
        response = client.post("/hmms/", json=MINIFAM_HMM)
        assert response.status_code == 201


def test_read_hmm(app):
    with TestClient(app) as client:
        response = client.post("/hmms/", json=MINIFAM_HMM)
        assert response.status_code == 201
        response = client.get("/hmms/1")
        assert response.status_code == 200


def test_read_hmms(app):
    with TestClient(app) as client:
        response = client.post("/hmms/", json=MINIFAM_HMM)
        assert response.status_code == 201
        response = client.get("/hmms")
        assert response.status_code == 200


def test_delete_hmm(app):
    with TestClient(app) as client:
        response = client.post("/hmms/", json=MINIFAM_HMM)
        assert response.status_code == 201
        response = client.delete("/hmms/1")
        assert response.status_code == 204
