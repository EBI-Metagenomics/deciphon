import json
from functools import lru_cache
from pathlib import Path

import requests

from deciphon.config import get_config
from deciphon.models import DBCreate, HMMCreate, ScanCreate, SnapCreate
from deciphon.storage import storage_has, storage_put

__all__ = ["API", "get_api"]


class API:
    @property
    def root(self):
        cfg = get_config()
        proto = cfg.api_proto
        host = cfg.api_host
        port = cfg.api_port
        prefix = cfg.api_prefix
        return f"{proto}://{host}:{port}{prefix}"

    @property
    def key(self):
        return {"X-API-Key": get_config().api_key}

    def _create_hmm(self, hmm: HMMCreate):
        r = requests.post(self.root + "/hmms/", json=hmm.dict(), headers=self.key)
        r.raise_for_status()

    def create_hmm(self, hmm: Path):
        if not storage_has(sha256sum(hmm)):
            storage_put(hmm)
        self._create_hmm(HMMCreate(sha256=sha256sum(hmm), filename=hmm.name))

    def read_hmms(self):
        r = requests.get(self.root + "/hmms")
        r.raise_for_status()
        return json.dumps(r.json())

    def read_hmm(self, hmm_id: int):
        r = requests.get(self.root + f"/hmms/{hmm_id}")
        r.raise_for_status()
        return json.dumps(r.json())

    def _create_db(self, db: DBCreate):
        r = requests.post(self.root + "/dbs/", json=db.dict(), headers=self.key)
        r.raise_for_status()

    def create_db(self, db: Path):
        if not storage_has(sha256sum(db)):
            storage_put(db)
        self._create_db(DBCreate(sha256=sha256sum(db), filename=db.name))

    def read_dbs(self):
        r = requests.get(self.root + "/dbs")
        r.raise_for_status()
        return json.dumps(r.json())

    def read_db(self, db_id: int):
        r = requests.get(self.root + f"/dbs/{db_id}")
        r.raise_for_status()
        return json.dumps(r.json())

    def create_scan(self, scan: ScanCreate):
        r = requests.post(self.root + "/scans/", json=scan.dict())
        r.raise_for_status()

    def read_scans(self):
        r = requests.get(self.root + "/scans")
        r.raise_for_status()
        return json.dumps(r.json())

    def read_scan(self, scan_id: int):
        r = requests.get(self.root + f"/scans/{scan_id}")
        r.raise_for_status()
        return json.dumps(r.json())

    def _create_snap(self, scan_id: int, snap: SnapCreate):
        url = self.root + f"/scans/{scan_id}/{snap.filename}"
        r = requests.put(url, json=snap.dict())
        r.raise_for_status()

    def create_snap(self, scan_id: int, snap: Path):
        if not storage_has(sha256sum(snap)):
            storage_put(snap)
        snap_create = SnapCreate(sha256=sha256sum(snap), filename=snap.name)
        self._create_snap(scan_id, snap_create)

    def read_snap(self, scan_id: int):
        r = requests.get(self.root + f"/scans/{scan_id}/snap.dcs")
        r.raise_for_status()
        return json.dumps(r.json())

    def read_jobs(self):
        r = requests.get(self.root + "/jobs")
        r.raise_for_status()
        return json.dumps(r.json())

    def read_job(self, job_id: int):
        r = requests.get(self.root + f"/jobs/{job_id}")
        r.raise_for_status()
        return json.dumps(r.json())

    def read_gff(self, scan_id: int):
        r = requests.get(self.root + f"/scans/{scan_id}/gff")
        r.raise_for_status()
        return r.text

    def read_codon(self, scan_id: int):
        r = requests.get(self.root + f"/scans/{scan_id}/codon")
        r.raise_for_status()
        return r.text

    def read_amino(self, scan_id: int):
        r = requests.get(self.root + f"/scans/{scan_id}/amino")
        r.raise_for_status()
        return r.text

    def read_state(self, scan_id: int):
        r = requests.get(self.root + f"/scans/{scan_id}/state")
        r.raise_for_status()
        return r.text

    def read_query(self, scan_id: int):
        r = requests.get(self.root + f"/scans/{scan_id}/query")
        r.raise_for_status()
        return r.text

    def read_align(self, scan_id: int):
        r = requests.get(self.root + f"/scans/{scan_id}/align")
        r.raise_for_status()
        return r.text


def sha256sum(filepath: Path):
    del filepath
    return ""
    # return CID.from_file(filepath, progress=Progress("Hash")).hex()


@lru_cache
def get_api() -> API:
    return API()
