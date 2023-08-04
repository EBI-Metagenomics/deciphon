from fastapi.testclient import TestClient

MINIFAM_HMM = {"name": "minifam.hmm"}


def test_hmm_not_found(app):
    with TestClient(app) as client:
        response = client.get("/hmms/1")
        assert response.status_code == 404


def test_create_hmm(app):
    with TestClient(app) as client:
        response = client.get("/hmms/presigned-upload")
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
