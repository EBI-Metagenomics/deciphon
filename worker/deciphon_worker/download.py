import shlex
import subprocess
from pathlib import Path

import requests
from loguru import logger
from pydantic import HttpUrl


def download(url: HttpUrl, dst: Path):
    logger.info(f"downloading {url}")
    u = url.unicode_string()
    try:
        cmd = shlex.join(["curl", "--silent", "-L", u]) + " > " + shlex.join([str(dst)])
        subprocess.check_call(cmd, shell=True)
    except subprocess.CalledProcessError:
        chunk_size = 4_194_304
        with requests.get(u, stream=True) as r:
            r.raise_for_status()
            with open(dst, "wb") as f:
                for chunk in r.iter_content(chunk_size=chunk_size):
                    f.write(chunk)
