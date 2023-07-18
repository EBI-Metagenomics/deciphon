from typing import List

from deciphon_api.models import Seq, Snap, SnapCreate
from deciphon_api.snap_validate import snap_validate

__all__ = ["create_snap"]


def create_snap(scan_id: int, seqs: List[Seq], snap: SnapCreate):
    snap_validate(seqs, snap)
    return Snap.from_orm(snap, update={"scan_id": scan_id})
