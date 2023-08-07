from fastapi.testclient import TestClient


def test_job_not_found(client: TestClient):
    assert client.get("/jobs/1").status_code == 404


def test_read_one(client: TestClient, files, s3_upload):
    upload_hmm(client, files, s3_upload)
    assert client.get("/jobs/1").status_code == 200


def test_read_many(client: TestClient, files, s3_upload):
    upload_hmm(client, files, s3_upload)
    response = client.get("/jobs")
    assert response.status_code == 200
    assert len(response.json()) == 1


def upload_hmm(client: TestClient, files, s3_upload):
    response = client.get("/hmms/presigned-upload/minifam.hmm")
    s3_upload(response.json(), files / "minifam.hmm")
    client.post("/hmms/", json={"name": "minifam.hmm"})
