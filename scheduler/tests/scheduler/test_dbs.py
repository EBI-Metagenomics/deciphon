from fastapi.testclient import TestClient


def test_db_not_found(client: TestClient):
    assert client.get("/dbs/1").status_code == 404


def test_presigned_url(client: TestClient, files, s3_upload):
    upload_hmm(client, files, s3_upload)
    assert client.get("/dbs/presigned-upload/minifam.dcp").status_code == 200


def test_create_failure(client: TestClient, files, s3_upload):
    upload_hmm(client, files, s3_upload)
    client.get("/dbs/presigned-upload/minifam.dcp")
    assert client.post("/dbs/", json={"name": "minifam.dcp"}).status_code == 422


def test_create_success(client: TestClient, files, s3_upload):
    upload_hmm(client, files, s3_upload)
    response = client.get("/dbs/presigned-upload/minifam.dcp")
    s3_upload(response.json(), files / "minifam.dcp")
    assert client.post("/dbs/", json={"name": "minifam.dcp"}).status_code == 201


def test_read_one(client: TestClient, files, s3_upload):
    upload_hmm(client, files, s3_upload)
    response = client.get("/dbs/presigned-upload/minifam.dcp")
    s3_upload(response.json(), files / "minifam.dcp")
    client.post("/dbs/", json={"name": "minifam.dcp"})
    assert client.get("/dbs/1").status_code == 200


def test_read_many(client: TestClient, files, s3_upload):
    upload_hmm(client, files, s3_upload)
    response = client.get("/dbs/presigned-upload/minifam.dcp")
    s3_upload(response.json(), files / "minifam.dcp")
    client.post("/dbs/", json={"name": "minifam.dcp"})
    assert client.get("/dbs").status_code == 200


def test_delete(client: TestClient, files, s3_upload):
    upload_hmm(client, files, s3_upload)
    response = client.get("/dbs/presigned-upload/minifam.dcp")
    s3_upload(response.json(), files / "minifam.dcp")
    client.post("/dbs/", json={"name": "minifam.dcp"})
    assert client.delete("/dbs/1").status_code == 204


def upload_hmm(client: TestClient, files, s3_upload):
    response = client.get("/hmms/presigned-upload/minifam.hmm")
    s3_upload(response.json(), files / "minifam.hmm")
    client.post("/hmms/", json={"name": "minifam.hmm"})
