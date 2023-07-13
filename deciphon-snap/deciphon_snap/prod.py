from pydantic import BaseModel, RootModel
from typing import List

from deciphon_snap.hmmer import H3Result
from deciphon_snap.match import LazyMatchList
from deciphon_snap.query_interval import QueryIntervalBuilder
from deciphon_snap.hit import HitList

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
    def hits(self):
        qibuilder = QueryIntervalBuilder(self.match_list)
        hits = []
        for hit in HitList.make(self.match_list):
            hit.interval = qibuilder.make(hit.match_list_interval)
            hit.match_list = self.match_list.evaluate()
            hits.append(hit)
        return hits

    @property
    def hmmer(self):
        assert self.h3result is not None
        return self.h3result

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
