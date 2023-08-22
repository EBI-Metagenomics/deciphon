from __future__ import annotations

from typing import List, overload

from pydantic import BaseModel, RootModel

from deciphon_snap.fasta import FASTAItem, FASTAList
from deciphon_snap.hit import Hit, HitList
from deciphon_snap.hmmer import H3Result
from deciphon_snap.match import LazyMatchList, Match, MatchElemName, MatchList
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

    @overload
    def __getitem__(self, i: int) -> Prod:
        ...

    @overload
    def __getitem__(self, i: slice) -> ProdList:
        ...

    def __getitem__(self, i: int | slice):
        if isinstance(i, slice):
            return ProdList.model_validate(self.root[i])
        prod = self.root[i]
        assert isinstance(prod, Prod)
        return prod

    def __iter__(self):
        return iter(self.root)

    def fasta_list(self, name: MatchElemName):
        return FASTAList(root=list(self._fasta_list(name)))

    def _fasta_list(self, name: MatchElemName):
        for x in self.root:
            if name == MatchElemName.QUERY:
                yield FASTAItem(defline=str(x.seq_id), sequence=x.match_list.query)
            elif name == MatchElemName.STATE:
                yield FASTAItem(defline=str(x.seq_id), sequence=x.match_list.state)
            elif name == MatchElemName.CODON:
                yield FASTAItem(defline=str(x.seq_id), sequence=x.match_list.codon)
            elif name == MatchElemName.AMINO:
                yield FASTAItem(defline=str(x.seq_id), sequence=x.match_list.amino)
            else:
                assert False
