from typing import List

from pydantic import BaseModel

from deciphon_snap.match import Match

__all__ = ["Hit"]


class Hit(BaseModel):
    id: int
    name: str
    prod_id: int
    evalue: float
    matchs: List[Match]
    feature_start: int = 0
    feature_end: int = 0
