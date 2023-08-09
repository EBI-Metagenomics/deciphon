import requests
from pydantic import HttpUrl

from deciphon_worker.settings import Settings


class Presigned:
    def __init__(self, settings: Settings):
        self._url = settings.scheduler_url.unicode_string()

    def _download_url(self, path: str):
        response = requests.get(f"{self._url}/{path}")
        response.raise_for_status()
        return HttpUrl(response.json()["url"])

    def download_hmm_url(self, filename: str):
        return self._download_url(f"hmms/presigned-download/{filename}")

    def download_db_url(self, filename: str):
        return self._download_url(f"dbs/presigned-download/{filename}")
