from pydantic import BaseModel

from deciphon_snap.hmmer import H3Result
from deciphon_snap.match_list import LazyMatchList

__all__ = ["Prod"]


class Prod(BaseModel):
    id: int
    seq_id: int
    profile: str
    abc: str
    alt: float
    null: float
    evalue: float
    match_list: LazyMatchList
    h3result: H3Result
