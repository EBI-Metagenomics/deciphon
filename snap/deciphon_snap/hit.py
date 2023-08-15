from __future__ import annotations

from typing import List, Type, TypeVar

from pydantic import BaseModel, RootModel

from deciphon_snap.match import MatchList, MatchListInterval
from deciphon_snap.query_interval import QueryInterval
from deciphon_snap.match import Match

__all__ = ["Hit", "HitList"]


class Hit(BaseModel):
    id: int
    match_list_interval: MatchListInterval
    _interval: QueryInterval | None = None
    _match_list: MatchList | None = None

    @property
    def interval(self):
        assert self._interval is not None
        return self._interval

    @interval.setter
    def interval(self, x: QueryInterval):
        self._interval = x

    @property
    def match_list(self):
        assert self._match_list is not None
        return self._match_list

    @match_list.setter
    def match_list(self, x: MatchList):
        self._match_list = x

    @property
    def matches(self):
        assert self._interval is not None
        assert self._match_list is not None
        matches: list[Match] = []
        offset = self._interval.pyinterval.start
        for x in self._match_list[self.match_list_interval.slice]:
            x.position = offset
            if x.state.startswith("I"):
                offset += len(x.query)
            if x.state.startswith("M"):
                offset += len(x.query)
            matches.append(x)
        return matches


T = TypeVar("T", bound="HitList")


class HitList(RootModel):
    root: List[Hit]

    def __len__(self):
        return len(self.root)

    def __getitem__(self, i):
        if isinstance(i, slice):
            return HitList.model_validate(self.root[i])
        hit = self.root[i]
        assert isinstance(hit, Hit)
        return hit

    def __iter__(self):
        return iter(self.root)

    def __str__(self):
        return " ".join(str(i) for i in self.root)

    @classmethod
    def make(cls: Type[T], match_list: MatchList) -> T:
        hits: List[Hit] = []

        offset = 0
        hit_start_found = False
        hit_end_found = False
        match_start = 0
        match_stop = 0

        for i, x in enumerate(match_list):
            if not hit_start_found and is_core_state(x.state):
                match_start = i
                hit_start_found = True

            if hit_start_found and not is_core_state(x.state):
                hit_end_found = True

            if hit_end_found:
                match_stop = i
                mi = MatchListInterval(start=match_start, stop=match_stop)
                hit_id = len(hits)
                hit = Hit(id=hit_id, match_list_interval=mi)
                hits.append(hit)
                hit_start_found = False
                hit_end_found = False

            offset += len(x.query)

        return cls.model_validate(hits)


def is_core_state(state: str):
    return state.startswith("M") or state.startswith("I") or state.startswith("D")
