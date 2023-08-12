import urllib.parse

import requests
from pydantic import HttpUrl


class Presigned:
    def __init__(self, sched_url: HttpUrl):
        self._url = sched_url.unicode_string()

    def _request(self, path: str):
        response = requests.get(urllib.parse.urljoin(self._url, path))
        response.raise_for_status()
        return response.json()

    def download_hmm_url(self, filename: str):
        x = self._request(f"hmms/presigned-download/{filename}")
        return HttpUrl(x["url"])

    def download_db_url(self, filename: str):
        x = self._request(f"dbs/presigned-download/{filename}")
        return HttpUrl(x["url"])

    def upload_db_post(self, filename: str):
        x = self._request(f"dbs/presigned-upload/{filename}")
        url = HttpUrl(x["url"])
        fields = x["fields"]
        return (url, fields)
