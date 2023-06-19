import pytest
from fastapi import FastAPI
from fastapi.testclient import TestClient

from deciphon_api.config import get_config

pytestmark = [pytest.mark.usefixtures("cleandir")]
HEADERS = {"X-API-Key": f"{get_config().key}"}


MINIFAM_HMM = {
    "sha256": "fe305d9c09e123f987f49b9056e34c374e085d8831f815cc73d8ea4cdec84960",
    "filename": "minifam.hmm",
}

MINIFAM_DCP = {
    "sha256": "40d96b5a62ff669e19571c392ab711c7188dd5490744edf6c66051ecb4f2243d",
    "filename": "minifam.dcp",
}


def url(path: str):
    return f"{get_config().prefix}{path}"


def test_no_auth_to_create_db(app: FastAPI):
    with TestClient(app) as client:
        response = client.post(url("/hmms/"), json=MINIFAM_HMM, headers=HEADERS)
        assert response.status_code == 201
        response = client.post(url("/dbs/"), json=MINIFAM_DCP)
        assert response.status_code == 403
        assert response.json() == {"detail": "Not authenticated"}


def test_db_not_found(app: FastAPI):
    with TestClient(app) as client:
        response = client.post(url("/hmms/"), json=MINIFAM_HMM, headers=HEADERS)
        assert response.status_code == 201
        response = client.get(url("/dbs/1"))
        assert response.status_code == 404


def test_create_db(app: FastAPI):
    with TestClient(app) as client:
        response = client.post(url("/hmms/"), json=MINIFAM_HMM, headers=HEADERS)
        assert response.status_code == 201
        response = client.post(url("/dbs/"), json=MINIFAM_DCP, headers=HEADERS)
        assert response.status_code == 201


def test_read_db(app: FastAPI):
    with TestClient(app) as client:
        response = client.post(url("/hmms/"), json=MINIFAM_HMM, headers=HEADERS)
        assert response.status_code == 201
        response = client.post(url("/dbs/"), json=MINIFAM_DCP, headers=HEADERS)
        assert response.status_code == 201
        response = client.get(url("/dbs/1"))
        assert response.status_code == 200


def test_read_dbs(app: FastAPI):
    with TestClient(app) as client:
        response = client.post(url("/hmms/"), json=MINIFAM_HMM, headers=HEADERS)
        assert response.status_code == 201
        response = client.post(url("/dbs/"), json=MINIFAM_DCP, headers=HEADERS)
        assert response.status_code == 201
        response = client.get(url("/dbs"))
        assert response.status_code == 200


def test_delete_db(app: FastAPI):
    with TestClient(app) as client:
        response = client.post(url("/hmms/"), json=MINIFAM_HMM, headers=HEADERS)
        assert response.status_code == 201
        response = client.post(url("/dbs/"), json=MINIFAM_DCP, headers=HEADERS)
        assert response.status_code == 201
        response = client.delete(url("/dbs/1"), headers=HEADERS)
        assert response.status_code == 204
