from typing import List

from pydantic import BaseModel, RootModel

from deciphon_snap.hit import Hit, HitList
from deciphon_snap.hmmer import H3Result
from deciphon_snap.match import Match
from deciphon_snap.match import LazyMatchList, MatchList
from deciphon_snap.query_interval import QueryIntervalBuilder

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
    def hits(self) -> list[Hit]:
        qibuilder = QueryIntervalBuilder(self.match_list.evaluate())
        hits = []
        for hit in HitList.make(self.match_list.evaluate()):
            hit.interval = qibuilder.make(hit.match_list_interval)
            hit.match_list = self.match_list.evaluate()
            hits.append(hit)
        return hits

    @property
    def matches(self):
        matches = []
        i = 0
        for x in self.match_list:
            match = Match.model_validate(x)
            match.position = i
            matches.append(match)
            i += len(match.query)
        return MatchList(root=matches)

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
