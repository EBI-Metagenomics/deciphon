from pathlib import Path

from blx.cid import CID
from blx.client import get_client as get_blx
from blx.progress import Progress

__all__ = ["storage_has", "storage_put", "storage_get"]


def storage_has(sha256: str) -> bool:
    return get_blx().has(CID(sha256))


def storage_put(filepath: Path):
    cid = CID.from_file(filepath, progress=Progress("Hash"))
    return get_blx().put(cid, filepath, progress=Progress("Upload"))


def storage_get(sha256: str, filepath: Path):
    return get_blx().get(CID(sha256), filepath, progress=Progress("Download"))
