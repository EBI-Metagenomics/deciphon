from pathlib import Path

import requests
from pydantic import HttpUrl

from deciphon_sched.logger import Logger
from deciphon_sched.settings import Settings
from deciphon_sched.storage import Storage


def test_storage(s3, settings: Settings, logger: Logger, tmp_path: Path):
    settings.s3_key = s3["access_key"]
    settings.s3_secret = s3["secret_key"]
    settings.s3_url = HttpUrl(s3["url"])
    storage = Storage(settings, logger)

    info = storage.presigned_upload("example.txt")

    with open(tmp_path / "example.txt", "w") as f:
        f.write("content")

    with open(tmp_path / "example.txt", "rb") as f:
        files = {"file": (str(tmp_path / "example.txt"), f)}
        url = info.url.unicode_string()
        response = requests.post(url, data=info.fields, files=files)
        assert response.ok
        assert response.status_code == 204
