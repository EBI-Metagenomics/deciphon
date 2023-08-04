from fastapi.testclient import TestClient

MINIFAM_HMM = {"name": "minifam.hmm"}
MINIFAM_DCP = {"name": "minifam.dcp"}


def test_db_not_found(app):
    with TestClient(app) as client:
        response = client.post("/hmms/", json=MINIFAM_HMM)
        assert response.status_code == 201
        response = client.get("/dbs/1")
        assert response.status_code == 404


def test_create_db(app):
    with TestClient(app) as client:
        response = client.post("/hmms/", json=MINIFAM_HMM)
        assert response.status_code == 201
        response = client.post("/dbs/", json=MINIFAM_DCP)
        assert response.status_code == 201


def test_read_db(app):
    with TestClient(app) as client:
        response = client.post("/hmms/", json=MINIFAM_HMM)
        assert response.status_code == 201
        response = client.post("/dbs/", json=MINIFAM_DCP)
        assert response.status_code == 201
        response = client.get("/dbs/1")
        assert response.status_code == 200


def test_read_dbs(app):
    with TestClient(app) as client:
        response = client.post("/hmms/", json=MINIFAM_HMM)
        assert response.status_code == 201
        response = client.post("/dbs/", json=MINIFAM_DCP)
        assert response.status_code == 201
        response = client.get("/dbs")
        assert response.status_code == 200


def test_delete_db(app):
    with TestClient(app) as client:
        response = client.post("/hmms/", json=MINIFAM_HMM)
        assert response.status_code == 201
        response = client.post("/dbs/", json=MINIFAM_DCP)
        assert response.status_code == 201
        response = client.delete("/dbs/1")
        assert response.status_code == 204
