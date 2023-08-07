from fastapi.testclient import TestClient

SCAN = {
    "db_id": 1,
    "multi_hits": True,
    "hmmer3_compat": True,
    "seqs": [{"name": "seq1", "data": "ACGT"}, {"name": "seq2", "data": "GTT"}],
}


def test_create(client: TestClient, files, s3_upload):
    upload_hmm(client, files, s3_upload)
    upload_db(client, files, s3_upload)
    assert client.post("/scans/", json=SCAN).status_code == 201


def test_read_one(client: TestClient, files, s3_upload):
    upload_hmm(client, files, s3_upload)
    upload_db(client, files, s3_upload)
    client.post("/scans/", json=SCAN)
    assert client.get("/scans/1").status_code == 200


def test_read_many(client: TestClient, files, s3_upload):
    upload_hmm(client, files, s3_upload)
    upload_db(client, files, s3_upload)
    client.post("/scans/", json=SCAN)
    assert client.get("/scans").status_code == 200


def test_delete(client: TestClient, files, s3_upload):
    upload_hmm(client, files, s3_upload)
    upload_db(client, files, s3_upload)
    client.post("/scans/", json=SCAN)
    assert client.delete("/scans/1").status_code == 204


def upload_hmm(client: TestClient, files, s3_upload):
    response = client.get("/hmms/presigned-upload/minifam.hmm")
    s3_upload(response.json(), files / "minifam.hmm")
    client.post("/hmms/", json={"name": "minifam.hmm"})


def upload_db(client: TestClient, files, s3_upload):
    response = client.get("/dbs/presigned-upload/minifam.dcp")
    s3_upload(response.json(), files / "minifam.dcp")
    client.post("/dbs/", json={"name": "minifam.dcp"})
