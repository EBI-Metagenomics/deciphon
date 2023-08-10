import requests
from fastapi.testclient import TestClient


def test_hmm_not_found(client: TestClient):
    assert client.get("/hmms/1").status_code == 404


def test_presigned_upload(client: TestClient):
    assert client.get("/hmms/presigned-upload/minifam.hmm").status_code == 200


def test_presigned_download_failure(client: TestClient):
    assert client.get("/hmms/presigned-download/minifam.hmm").status_code == 422


def test_create(client: TestClient, files, s3_upload):
    response = client.get("/hmms/presigned-upload/minifam.hmm")
    s3_upload(response.json(), files / "minifam.hmm")
    assert upload_hmm(client, s3_upload, files / "minifam.hmm").status_code == 201


def test_download(client: TestClient, files, s3_upload):
    upload_hmm(client, s3_upload, files / "minifam.hmm")
    url = client.get("/hmms/presigned-download/minifam.hmm").json()["url"]
    response = requests.get(url)
    assert response.status_code == 200
    assert hash(open(files / "minifam.hmm", "rb").read()) == hash(response.text)


def test_read_one(client: TestClient, files, s3_upload):
    upload_hmm(client, s3_upload, files / "minifam.hmm")
    assert client.get("/hmms/1").status_code == 200


def test_read_many(client: TestClient, files, s3_upload):
    upload_hmm(client, s3_upload, files / "minifam.hmm")
    assert client.get("/hmms").status_code == 200


def test_delete(client: TestClient, files, s3_upload):
    upload_hmm(client, s3_upload, files / "minifam.hmm")
    assert client.delete("/hmms/1").status_code == 204


def upload_hmm(client: TestClient, s3_upload, file):
    response = client.get(f"/hmms/presigned-upload/{file.name}")
    s3_upload(response.json(), file)
    params = {"gencode": 1, "epsilon": 0.01}
    return client.post("/hmms/", params=params, json={"name": file.name})
