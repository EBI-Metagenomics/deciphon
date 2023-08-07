from pathlib import Path
import requests


def s3_upload(presigned_upload, file: Path):
    with open(file, "rb") as f:
        files = {"file": (file.name, f)}
        url = presigned_upload["url"]
        fields = presigned_upload["fields"]
        return requests.post(url, data=fields, files=files)
