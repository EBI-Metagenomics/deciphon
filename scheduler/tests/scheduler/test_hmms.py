from fastapi.testclient import TestClient


def test_hmm_not_found(client: TestClient):
    assert client.get("/hmms/1").status_code == 404


def test_presigned_upload(client: TestClient):
    assert client.get("/hmms/presigned-upload/minifam.hmm").status_code == 200


def test_create_failure(client: TestClient):
    client.get("/hmms/presigned-upload/minifam.hmm")
    assert client.post("/hmms/", json={"name": "minifam.hmm"}).status_code == 422


def test_create_success(client: TestClient, files, s3_upload):
    response = client.get("/hmms/presigned-upload/minifam.hmm")
    s3_upload(response.json(), files / "minifam.hmm")
    assert client.post("/hmms/", json={"name": "minifam.hmm"}).status_code == 201


def test_read_one(client: TestClient, files, s3_upload):
    upload_hmm(client, files, s3_upload)
    assert client.get("/hmms/1").status_code == 200


def test_read_many(client: TestClient, files, s3_upload):
    upload_hmm(client, files, s3_upload)
    assert client.get("/hmms").status_code == 200


def test_delete(client: TestClient, files, s3_upload):
    upload_hmm(client, files, s3_upload)
    assert client.delete("/hmms/1").status_code == 204


def upload_hmm(client: TestClient, files, s3_upload):
    response = client.get("/hmms/presigned-upload/minifam.hmm")
    s3_upload(response.json(), files / "minifam.hmm")
    client.post("/hmms/", json={"name": "minifam.hmm"})
