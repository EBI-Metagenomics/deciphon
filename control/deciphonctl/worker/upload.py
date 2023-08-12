from pathlib import Path

import requests
from loguru import logger
from pydantic import HttpUrl


def upload(file: Path, url: HttpUrl, fields):
    u = url.unicode_string()
    logger.info(f"uploading {file} to {u}")
    with open(file, "rb") as f:
        files = {"file": (file.name, f)}
        response = requests.post(u, data=fields, files=files)
        response.raise_for_status()
