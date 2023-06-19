from blx.cid import CID
from blx.app import BLXApp

__all__ = ["storage_has"]


def storage_has(sha256: str) -> bool:
    return BLXApp().has(CID(sha256))
