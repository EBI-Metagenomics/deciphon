import requests
from pathlib import Path

from deciphon_scheduler.settings import Settings
from deciphon_scheduler.storage import Storage


def test_storage(s3server, tmp_path: Path):
    settings = Settings(
        s3_url=s3server["url"],
        s3_key=s3server["key"],
        s3_secret=s3server["secret"],
        s3_bucket=s3server["bucket"],
    )
    storage = Storage(settings)

    info = storage.presigned_upload("example.txt")

    with open(tmp_path / "example.txt", "w") as f:
        f.write("content")

    with open(tmp_path / "example.txt", "rb") as f:
        files = {"file": (str(tmp_path / "example.txt"), f)}
        response = requests.post(info["url"], data=info["fields"], files=files)
        assert response.ok
        assert response.status_code == 204
