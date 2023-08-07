from fastapi.testclient import TestClient


def test_hmm_not_found(compose):
    with TestClient(compose["app"]) as client:
        assert client.get("/hmms/1").status_code == 404


def test_presigned_hmm_url(compose):
    with TestClient(compose["app"]) as client:
        assert client.get("/hmms/presigned-upload/minifam.hmm").status_code == 200


def test_create_hmm_failure(compose):
    with TestClient(compose["app"]) as client:
        client.get("/hmms/presigned-upload/minifam.hmm")
        assert client.post("/hmms/", json={"name": "minifam.hmm"}).status_code == 422


def test_create_hmm(compose, files, s3_upload):
    with TestClient(compose["app"]) as client:
        response = client.get("/hmms/presigned-upload/minifam.hmm")
        s3_upload(response.json(), files / "minifam.hmm")
        assert client.post("/hmms/", json={"name": "minifam.hmm"}).status_code == 201


def test_read_hmm(compose, files, s3_upload):
    with TestClient(compose["app"]) as client:
        response = client.get("/hmms/presigned-upload/minifam.hmm")
        s3_upload(response.json(), files / "minifam.hmm")
        client.post("/hmms/", json={"name": "minifam.hmm"})
        assert client.get("/hmms/1").status_code == 200


def test_read_hmms(compose, files, s3_upload):
    with TestClient(compose["app"]) as client:
        response = client.get("/hmms/presigned-upload/minifam.hmm")
        s3_upload(response.json(), files / "minifam.hmm")
        client.post("/hmms/", json={"name": "minifam.hmm"})
        assert client.get("/hmms").status_code == 200


def test_delete_hmm(compose, files, s3_upload):
    with TestClient(compose["app"]) as client:
        response = client.get("/hmms/presigned-upload/minifam.hmm")
        s3_upload(response.json(), files / "minifam.hmm")
        client.post("/hmms/", json={"name": "minifam.hmm"})
        assert client.delete("/hmms/1").status_code == 204
