from pydantic import BaseModel, RootModel
from typing import List

from deciphon_snap.hmmer import H3Result
from deciphon_snap.match import LazyMatchList

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
    h3result: H3Result | None = None

    @property
    def query(self):
        return self.match_list.query

    @property
    def codon(self):
        return self.match_list.codon

    @property
    def amino(self):
        return self.match_list.amino


class ProdList(RootModel):
    root: List[Prod]

    def __len__(self):
        return len(self.root)

    def __getitem__(self, i) -> Prod:
        return self.root[i]

    def __iter__(self):
        return iter(self.root)
