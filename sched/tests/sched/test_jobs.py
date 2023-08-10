from fastapi.testclient import TestClient


def test_job_not_found(client: TestClient):
    assert client.get("/jobs/1").status_code == 404


def test_read_one(client: TestClient, files, s3_upload):
    upload_hmm(client, s3_upload, files / "minifam.hmm")
    assert client.get("/jobs/1").status_code == 200


def test_read_many(client: TestClient, files, s3_upload):
    upload_hmm(client, s3_upload, files / "minifam.hmm")
    response = client.get("/jobs")
    assert response.status_code == 200
    assert len(response.json()) == 1


def upload_hmm(client: TestClient, s3_upload, file):
    response = client.get(f"/hmms/presigned-upload/{file.name}")
    s3_upload(response.json(), file)
    params = {"gencode": 1, "epsilon": 0.01}
    return client.post("/hmms/", params=params, json={"name": file.name})
