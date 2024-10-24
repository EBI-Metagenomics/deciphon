from __future__ import annotations

import urllib.parse
from pathlib import Path
from typing import Callable, Optional

import requests
from deciphon_core.schema import DBName, Gencode, HMMName
from pydantic import FilePath, HttpUrl, TypeAdapter
from requests_toolbelt import MultipartEncoder, MultipartEncoderMonitor

from deciphon_poster.errors import PosterHTTPError
from deciphon_poster.schema import JobUpdate, Scan, UploadPost


class Poster:
    def __init__(self, sched_url: HttpUrl, s3_url: Optional[HttpUrl]):
        self.sched_url = sched_url
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

    def upload(
        self, file: Path, post: UploadPost, callback: Callable[[int, int]] | None = None
    ):
        with open(file, "rb") as f:
            fields = post.fields
            fields["file"] = (file.name, f)
            encoder = MultipartEncoder(fields=fields)
            if callback is None:
                data = MultipartEncoderMonitor(encoder)
            else:
                callback(int(encoder.len), 0)
                data = MultipartEncoderMonitor(
                    encoder,
                    lambda monitor: callback(int(encoder.len), monitor.bytes_read),
                )
            self.post(
                post.url_string,
                data=data,
                headers={
                    "content-type": encoder.content_type,
                    "content-length": str(encoder.len),
                },
            )

    def hmm_post(self, file: HMMName, gencode: Gencode, epsilon: float):
        self.post(
            self.url("hmms/"),
            params={"gencode": gencode, "epsilon": epsilon},
            json={"name": file.name},
        )

    def hmm_delete(self, hmm_id: int):
        self.delete(self.url(f"hmms/{hmm_id}"))

    def hmm_list(self):
        return self.get(self.url("hmms")).json()

    def db_post(self, file: DBName):
        self.post(self.url("dbs/"), json={"name": file.name})

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
        return urllib.parse.urljoin(self.sched_url.unicode_string(), endpoint)

    def _request(self, path: str):
        return self.get(self.url(path)).json()

    def download_hmm_url(self, filename: str):
        x = self._request(f"hmms/presigned-download/{filename}")
        return http_url(x["url"])

    def download_db_url(self, filename: str):
        x = self._request(f"dbs/presigned-download/{filename}")
        return http_url(x["url"])

    def upload_hmm_post(self, filename: str):
        x = self._request(f"hmms/presigned-upload/{filename}")
        url = self.s3_url if self.s3_url else http_url(x["url"])
        return UploadPost(url=url, fields=x["fields"])

    def upload_db_post(self, filename: str):
        x = self._request(f"dbs/presigned-upload/{filename}")
        url = self.s3_url if self.s3_url else http_url(x["url"])
        return UploadPost(url=url, fields=x["fields"])


def strip_empty_lines(s):
    lines = s.splitlines()
    while lines and not lines[0].strip():
        lines.pop(0)
    while lines and not lines[-1].strip():
        lines.pop()
    return "\n".join(lines)


def http_url(url: str) -> HttpUrl:
    return TypeAdapter(HttpUrl).validate_strings(url)
