from __future__ import annotations

import urllib.parse
from pathlib import Path
from typing import Optional

import requests
from deciphon_core.schema import Gencode
from pydantic import FilePath, HttpUrl
from requests_toolbelt import MultipartEncoder

from deciphon_poster.errors import PosterHTTPError
from deciphon_poster.models import DBFile, HMMFile, JobUpdate, Scan, UploadPost
from deciphon_poster.url import http_url


class Poster:
    def __init__(self, url: HttpUrl, s3_url: Optional[HttpUrl]):
        self._url = url
        self.s3_url = s3_url

    def handle_http_response(self, response):
        if not response.ok:
            raise PosterHTTPError(response)

    def get(self, url: str, params=None):
        response = requests.get(url, params=params)
        self.handle_http_response(response)
        return response

    def post(self, url: str, data=None, json=None, params=None, headers=None):
        r = requests.post(url, data=data, json=json, params=params, headers=headers)
        self.handle_http_response(r)
        return r

    def patch(self, url: str, data=None, json=None):
        response = requests.patch(url, data=data, json=json)
        self.handle_http_response(response)
        return response

    def delete(self, url: str, **kwargs):
        self.handle_http_response(requests.delete(url, **kwargs))

    def upload(self, file: Path, post: UploadPost):
        with open(file, "rb") as f:
            fields = post.fields
            fields["file"] = (file.name, f)
            m = MultipartEncoder(fields=fields)
            self.post(post.url_string, data=m, headers={"content-type": m.content_type})

    def hmm_post(self, file: HMMFile, gencode: Gencode, epsilon: float):
        self.post(
            self.url("hmms/"),
            params={"gencode": gencode, "epsilon": epsilon},
            json={"name": file.name},
        )

    def hmm_delete(self, hmm_id: int):
        self.delete(self.url(f"hmms/{hmm_id}"))

    def hmm_list(self):
        return self.get(self.url("hmms")).json()

    def db_post(self, file: DBFile):
        self.post(
            self.url("dbs/"),
            json={
                "name": file.name,
                "gencode": int(file.gencode),
                "epsilon": file.epsilon,
            },
        )

    def db_delete(self, db_id: int):
        self.delete(self.url(f"dbs/{db_id}"))

    def db_list(self):
        return self.get(self.url("dbs")).json()

    def job_list(self):
        return self.get(self.url("jobs")).json()

    def scan_post(self, scan: Scan):
        self.post(self.url("scans/"), json=scan.model_dump())

    def scan_delete(self, scan_id: int):
        self.delete(self.url(f"scans/{scan_id}"))

    def scan_list(self):
        return self.get(self.url("scans")).json()

    def job_patch(self, x: JobUpdate):
        json = {"state": x.state.value, "progress": x.progress, "error": x.error}
        self.patch(self.url(f"jobs/{x.id}"), json=json)

    def seq_list(self):
        return self.get(self.url("seqs")).json()

    def snap_post(self, scan_id: int, snap: FilePath):
        post = UploadPost(
            url=http_url(self.url(f"scans/{scan_id}/snap.dcs")), fields={}
        )
        self.upload(snap, post)

    def snap_get(self, scan_id: int):
        return self.get(self.url(f"scans/{scan_id}/snap.dcs")).content

    def snap_delete(self, scan_id: int):
        self.delete(self.url(f"scans/{scan_id}/snap.dcs"))

    def snap_view(self, scan_id: int):
        x = self.get(self.url(f"scans/{scan_id}/snap.dcs/view")).text
        return strip_empty_lines(x)

    def url(self, endpoint: str):
        return urllib.parse.urljoin(self._url.unicode_string(), endpoint)


def strip_empty_lines(s):
    lines = s.splitlines()
    while lines and not lines[0].strip():
        lines.pop(0)
    while lines and not lines[-1].strip():
        lines.pop()
    return "\n".join(lines)
