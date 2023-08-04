from fastapi.testclient import TestClient

MINIFAM_HMM = {"name": "minifam.hmm"}
MINIFAM_DCP = {"name": "minifam.dcp"}
SEQS = [{"name": "seq1", "data": "ACGT"}, {"name": "seq2", "data": "GTT"}]
SCAN = {
    "db_id": 1,
    "multi_hits": True,
    "hmmer3_compat": True,
    "seqs": SEQS,
}


def test_create_scan(app):
    with TestClient(app) as client:
        response = client.post("/hmms/", json=MINIFAM_HMM)
        assert response.status_code == 201
        response = client.post("/dbs/", json=MINIFAM_DCP)
        assert response.status_code == 201
        response = client.post("/scans/", json=SCAN)
        assert response.status_code == 201


def test_read_scan(app):
    with TestClient(app) as client:
        response = client.post("/hmms/", json=MINIFAM_HMM)
        assert response.status_code == 201
        response = client.post("/dbs/", json=MINIFAM_DCP)
        assert response.status_code == 201
        response = client.post("/scans/", json=SCAN)
        assert response.status_code == 201
        response = client.get("/scans/1")
        assert response.status_code == 200


def test_read_scans(app):
    with TestClient(app) as client:
        response = client.post("/hmms/", json=MINIFAM_HMM)
        assert response.status_code == 201
        response = client.post("/dbs/", json=MINIFAM_DCP)
        assert response.status_code == 201
        response = client.post("/scans/", json=SCAN)
        assert response.status_code == 201
        response = client.get("/scans")
        assert response.status_code == 200


def test_delete_scan(app):
    with TestClient(app) as client:
        response = client.post("/hmms/", json=MINIFAM_HMM)
        assert response.status_code == 201
        response = client.post("/dbs/", json=MINIFAM_DCP)
        assert response.status_code == 201
        response = client.post("/scans/", json=SCAN)
        assert response.status_code == 201
        response = client.delete("/scans/1")
        assert response.status_code == 204
