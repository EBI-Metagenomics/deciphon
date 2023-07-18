import importlib.resources
from pathlib import Path

from blx.download import download
from deciphon_api.cache_dir import get_cache_dir
from blx.cid import CID

__all__ = ["get_blob"]


def get_blob(name: str) -> Path:
    file = importlib.resources.files("deciphon_api").joinpath("blob.sha256")
    xy = (x.strip().split() for x in file.open("r").readlines())
    registry = {name: value for value, name in xy}
    dst = get_cache_dir() / name
    download(CID(registry[name]), dst, show_progress=False)
    return dst
