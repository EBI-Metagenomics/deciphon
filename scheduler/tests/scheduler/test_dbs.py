import requests
from fastapi.testclient import TestClient


def test_db_not_found(client: TestClient):
    assert client.get("/dbs/1").status_code == 404


def test_presigned_upload(client: TestClient, files, s3_upload):
    upload_hmm(client, s3_upload, files / "minifam.hmm")
    assert client.get("/dbs/presigned-upload/minifam.dcp").status_code == 200


def test_presigned_download_failure(client: TestClient, files, s3_upload):
    upload_hmm(client, s3_upload, files / "minifam.hmm")
    assert client.get("/dbs/presigned-download/minifam.dcp").status_code == 422


def test_create_failure(client: TestClient, files, s3_upload):
    upload_hmm(client, s3_upload, files / "minifam.hmm")
    client.get("/dbs/presigned-upload/minifam.dcp")
    assert client.post("/dbs/", json={"name": "minifam.dcp"}).status_code == 422


def test_create_success(client: TestClient, files, s3_upload):
    upload_hmm(client, s3_upload, files / "minifam.hmm")
    response = client.get("/dbs/presigned-upload/minifam.dcp")
    s3_upload(response.json(), files / "minifam.dcp")
    assert client.post("/dbs/", json={"name": "minifam.dcp"}).status_code == 201


def test_download(client: TestClient, files, s3_upload):
    upload_hmm(client, s3_upload, files / "minifam.hmm")
    upload_db(client, s3_upload, files / "minifam.dcp")
    url = client.get("/dbs/presigned-download/minifam.dcp").json()["url"]
    response = requests.get(url)
    assert response.status_code == 200
    assert hash(open(files / "minifam.dcp", "rb").read()) == hash(response.text)


def test_read_one(client: TestClient, files, s3_upload):
    upload_hmm(client, s3_upload, files / "minifam.hmm")
    upload_db(client, s3_upload, files / "minifam.dcp")
    assert client.get("/dbs/1").status_code == 200


def test_read_many(client: TestClient, files, s3_upload):
    upload_hmm(client, s3_upload, files / "minifam.hmm")
    upload_db(client, s3_upload, files / "minifam.dcp")
    assert client.get("/dbs").status_code == 200


def test_delete(client: TestClient, files, s3_upload):
    upload_hmm(client, s3_upload, files / "minifam.hmm")
    upload_db(client, s3_upload, files / "minifam.dcp")
    assert client.delete("/dbs/1").status_code == 204


def upload_hmm(client: TestClient, s3_upload, file):
    response = client.get(f"/hmms/presigned-upload/{file.name}")
    s3_upload(response.json(), file)
    client.post("/hmms/", json={"name": file.name})


def upload_db(client: TestClient, s3_upload, file):
    response = client.get(f"/dbs/presigned-upload/{file.name}")
    s3_upload(response.json(), file)
    client.post("/dbs/", json={"name": file.name, "gencode": 1, "epsilon": 0.01})
