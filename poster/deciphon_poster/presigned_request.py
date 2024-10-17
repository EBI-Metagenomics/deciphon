from deciphon_poster.models import UploadPost
from deciphon_poster.poster import Poster
from deciphon_poster.url import http_url


class PresignedRequest:
    def __init__(self, poster: Poster):
        self._poster = poster

    def _request(self, path: str):
        return self._poster.get(self._poster.url(path)).json()

    def download_hmm_url(self, filename: str):
        x = self._request(f"hmms/presigned-download/{filename}")
        return http_url(x["url"])

    def download_db_url(self, filename: str):
        x = self._request(f"dbs/presigned-download/{filename}")
        return http_url(x["url"])

    def upload_hmm_post(self, filename: str):
        x = self._request(f"hmms/presigned-upload/{filename}")
        url = self._poster.s3_url if self._poster.s3_url else http_url(x["url"])
        return UploadPost(url=url, fields=x["fields"])

    def upload_db_post(self, filename: str):
        x = self._request(f"dbs/presigned-upload/{filename}")
        url = self._poster.s3_url if self._poster.s3_url else http_url(x["url"])
        return UploadPost(url=url, fields=x["fields"])
