from fastapi.testclient import TestClient

SCAN = {
    "db_id": 1,
    "multi_hits": True,
    "hmmer3_compat": True,
    "seqs": [{"name": "seq1", "data": "ACGT"}, {"name": "seq2", "data": "GTT"}],
}


def test_upload(client: TestClient, files, s3_upload):
    upload_hmm(client, s3_upload, files / "minifam.hmm")
    upload_db(client, s3_upload, files / "minifam.dcp")
    client.post("/scans/", json=SCAN)
    files = {"file": open(files / "consensus.dcs", "rb")}
    assert client.post("/scans/1/snap.dcs", files=files).status_code == 201


def test_download(client: TestClient, files, s3_upload):
    upload_hmm(client, s3_upload, files / "minifam.hmm")
    upload_db(client, s3_upload, files / "minifam.dcp")
    client.post("/scans/", json=SCAN)
    files = {"file": open(files / "consensus.dcs", "rb")}
    client.post("/scans/1/snap.dcs", files=files)
    assert client.get("/scans/1/snap.dcs").status_code == 200


def test_delete(client: TestClient, files, s3_upload):
    upload_hmm(client, s3_upload, files / "minifam.hmm")
    upload_db(client, s3_upload, files / "minifam.dcp")
    client.post("/scans/", json=SCAN)
    files = {"file": open(files / "consensus.dcs", "rb")}
    client.post("/scans/1/snap.dcs", files=files)
    assert client.delete("/scans/1/snap.dcs").status_code == 204


def upload_hmm(client: TestClient, s3_upload, file):
    response = client.get(f"/hmms/presigned-upload/{file.name}")
    s3_upload(response.json(), file)
    params = {"gencode": 1, "epsilon": 0.01}
    return client.post("/hmms/", params=params, json={"name": file.name})


def test_view(client: TestClient, files, s3_upload):
    upload_hmm(client, s3_upload, files / "minifam.hmm")
    upload_db(client, s3_upload, files / "minifam.dcp")
    client.post("/scans/", json=SCAN)
    files = {"file": open(files / "consensus.dcs", "rb")}
    client.post("/scans/1/snap.dcs", files=files)
    assert client.get("/scans/1/snap.dcs/view").status_code == 200


def upload_db(client: TestClient, s3_upload, file):
    response = client.get(f"/dbs/presigned-upload/{file.name}")
    s3_upload(response.json(), file)
    return client.post("/dbs/", json={"name": file.name, "gencode": 1, "epsilon": 0.01})
