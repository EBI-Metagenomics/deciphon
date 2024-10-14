from functools import partial

from fastapi.testclient import TestClient

SCAN = {
    "db_id": 1,
    "multi_hits": True,
    "hmmer3_compat": True,
    "seqs": [{"name": "seq1", "data": "ACGT"}, {"name": "seq2", "data": "GTT"}],
}


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


def test_read_limit(client: TestClient, files, s3_upload):
    upload_hmm(client, s3_upload, files / "minifam.hmm")
    upload_db(client, s3_upload, files / "minifam.dcp")
    assert client.post("/scans/", json=SCAN).status_code == 201
    response = client.get("/jobs")
    assert response.status_code == 200
    assert len(response.json()) == 2
    response = client.get("/jobs", params={"limit": 3})
    assert response.status_code == 200
    assert len(response.json()) == 2
    response = client.get("/jobs", params={"limit": 2})
    assert response.status_code == 200
    assert len(response.json()) == 2
    response = client.get("/jobs", params={"limit": 1})
    assert response.status_code == 200
    assert len(response.json()) == 1
    response = client.get("/jobs", params={"limit": 0})
    assert response.status_code == 200
    assert len(response.json()) == 0


def test_update_done(client: TestClient, files, s3_upload):
    upload_hmm(client, s3_upload, files / "minifam.hmm")
    x = partial(client.patch, "/jobs/1")
    assert x(json={"state": "run", "progress": 0, "error": ""}).status_code == 200
    assert x(json={"state": "run", "progress": 100, "error": ""}).status_code == 200
    assert x(json={"state": "done", "progress": 100, "error": ""}).status_code == 200
    assert x(json={"state": "fail", "progress": 0, "error": ""}).status_code == 400


def test_update_fail(client: TestClient, files, s3_upload):
    upload_hmm(client, s3_upload, files / "minifam.hmm")
    x = partial(client.patch, "/jobs/1")
    assert x(json={"state": "run", "progress": 0, "error": ""}).status_code == 200
    assert x(json={"state": "run", "progress": 100, "error": ""}).status_code == 200
    assert x(json={"state": "fail", "progress": 0, "error": "msg"}).status_code == 200
    assert x(json={"state": "done", "progress": 100, "error": ""}).status_code == 400


def upload_hmm(client: TestClient, s3_upload, file):
    response = client.get(f"/hmms/presigned-upload/{file.name}")
    s3_upload(response.json(), file)
    params = {"gencode": 1, "epsilon": 0.01}
    return client.post("/hmms/", params=params, json={"name": file.name})


def upload_db(client: TestClient, s3_upload, file):
    response = client.get(f"/dbs/presigned-upload/{file.name}")
    s3_upload(response.json(), file)
    return client.post("/dbs/", json={"name": file.name, "gencode": 1, "epsilon": 0.01})
