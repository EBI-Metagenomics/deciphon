from __future__ import annotations

import urllib.parse
from pathlib import Path
from typing import Any

import requests
from deciphon_core.schema import Gencode
from loguru import logger
from pydantic import BaseModel, HttpUrl
from requests.models import HTTPError

from deciphonctl.models import DBFile, HMMFile, JobUpdate, Scan


class SchedHTTPError(HTTPError):
    def __init__(self, response):
        try:
            response.raise_for_status()
            assert False
        except HTTPError as x:
            msg = x.args[0]
            try:
                info = response.json()
            except requests.JSONDecodeError:
                info = response.text
            super().__init__(msg + f" returned: {info}", response=response)


class Sched:
    def __init__(self, url: HttpUrl):
        self._url = url

    def handle_http_response(self, response):
        logger.debug(f"{response.request} {response.request.url} {response}")
        if not response.ok:
            raise SchedHTTPError(response)

    def get(self, url, params=None, **kwargs):
        response = requests.get(url, params=params, **kwargs)
        self.handle_http_response(response)
        return response

    def post(self, url: str, data=None, json=None, **kwargs):
        response = requests.post(url, data=data, json=json, **kwargs)
        self.handle_http_response(response)
        return response

    def patch(self, url: str, data=None, **kwargs):
        response = requests.patch(url, data=data, **kwargs)
        self.handle_http_response(response)
        return response

    def delete(self, url: str, **kwargs):
        self.handle_http_response(requests.delete(url, **kwargs))

    @property
    def presigned(self):
        return Presigned(self)

    def upload(self, file: Path, post: UploadPost):
        logger.info(f"uploading {file} to {post.url_string}")
        with open(file, "rb") as f:
            files = {"file": (file.name, f)}
            self.post(post.url_string, data=post.fields, files=files)

    def hmm_post(self, file: HMMFile, gencode: Gencode, epsilon: float):
        self.post(
            self.url("/hmms/"),
            params={"gencode": gencode, "epsilon": epsilon},
            json={"name": file.name},
        )

    def hmm_delete(self, hmm_id: int):
        self.delete(self.url(f"/hmms/{hmm_id}"))

    def hmm_list(self):
        return self.get(self.url("/hmms")).json()

    def db_post(self, file: DBFile):
        self.post(
            self.url("/dbs/"),
            json={
                "name": file.name,
                "gencode": int(file.gencode),
                "epsilon": file.epsilon,
            },
        )

    def db_delete(self, db_id: int):
        self.delete(self.url(f"/dbs/{db_id}"))

    def db_list(self):
        return self.get(self.url("/dbs")).json()

    def job_list(self):
        return self.get(self.url("/jobs")).json()

    def scan_post(self, scan: Scan):
        self.post(self.url("/scans/"), json=scan.model_dump())

    def scan_delete(self, scan_id: int):
        self.delete(self.url(f"/scans/{scan_id}"))

    def scan_list(self):
        self.get(self.url("/scans")).json()

    def job_patch(self, x: JobUpdate):
        json = {"state": x.state.value, "progress": x.progress, "error": x.error}
        self.patch(self.url(f"jobs/{x.id}"), json=json)

    def url(self, endpoint: str):
        return urllib.parse.urljoin(self._url.unicode_string(), endpoint)


class Presigned:
    def __init__(self, sched: Sched):
        self._sched = sched

    def _request(self, path: str):
        return self._sched.get(self._sched.url(path)).json()

    def download_hmm_url(self, filename: str):
        x = self._request(f"hmms/presigned-download/{filename}")
        return HttpUrl(x["url"])

    def download_db_url(self, filename: str):
        x = self._request(f"dbs/presigned-download/{filename}")
        return HttpUrl(x["url"])

    def upload_hmm_post(self, filename: str):
        x = self._request(f"hmms/presigned-upload/{filename}")
        return UploadPost(url=HttpUrl(x["url"]), fields=x["fields"])

    def upload_db_post(self, filename: str):
        x = self._request(f"dbs/presigned-upload/{filename}")
        return UploadPost(url=HttpUrl(x["url"]), fields=x["fields"])


class UploadPost(BaseModel):
    url: HttpUrl
    fields: dict[str, Any]

    @property
    def url_string(self):
        return self.url.unicode_string()