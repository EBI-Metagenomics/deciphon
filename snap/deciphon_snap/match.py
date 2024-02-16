from __future__ import annotations

from dataclasses import dataclass
from enum import Enum
from functools import lru_cache
from typing import List, overload

from deciphon_intervals import PyInterval
from pydantic import BaseModel, ConfigDict

from deciphon_snap.amino import AminoInterval

__all__ = ["Match", "MatchList", "LazyMatchList", "MatchListInterval", "MatchElemName"]


class MatchElemName(Enum):
    QUERY = 1
    STATE = 2
    CODON = 3
    AMINO = 4


@dataclass(slots=True, frozen=True)
class Match:
    raw: str
    start: int
    end: int
    position: int = -1

    @classmethod
    def from_string(cls, x: str):
        return cls(raw=x, start=0, end=len(x))

    @property
    def query(self):
        start = self.start
        return self.raw[start : self.raw.find(",", start, self.end)]

    @property
    def state(self):
        i = self.start
        i = self.raw.find(",", i, self.end) + 1
        return self.raw[i : self.raw.find(",", i, self.end)]

    @property
    def codon(self):
        i = self.start
        i = self.raw.find(",", i, self.end) + 1
        i = self.raw.find(",", i, self.end) + 1
        return self.raw[i : self.raw.find(",", i, self.end)]

    @property
    def amino(self):
        i = self.start
        i = self.raw.find(",", i, self.end) + 1
        i = self.raw.find(",", i, self.end) + 1
        i = self.raw.find(",", i, self.end) + 1
        return self.raw[i : self.end]

    @property
    def query_size(self) -> int:
        return self.raw.find(",", self.start, self.end) - self.start

    @property
    def _state_symbol(self):
        i = self.start
        i = self.raw.find(",", i, self.end) + 1
        return self.raw[i]

    @property
    def is_insert_state(self):
        return self._state_symbol == "I"

    @property
    def is_match_state(self):
        return self._state_symbol == "M"

    @property
    def is_delete_state(self):
        return self._state_symbol == "D"

    @property
    def is_core_state(self):
        x = self._state_symbol
        return x == "I" or x == "M" or x == "D"

    def __str__(self):
        query = self.query if len(self.query) > 0 else "∅"
        state = self.state
        codon = self.codon if len(self.codon) > 0 else "∅"
        amino = self.amino if len(self.amino) > 0 else "∅"
        return f"({query},{state},{codon},{amino})"


@dataclass(slots=True, frozen=True)
class MatchList:
    root: List[Match]

    @classmethod
    def from_string(cls, x: str):
        y = [i for i in ifind(x, ";")]
        return cls([Match(raw=x, start=i[0], end=i[1]) for i in y])

    def __len__(self):
        return len(self.root)

    @overload
    def __getitem__(self, i: int) -> Match:
        ...

    @overload
    def __getitem__(self, i: slice) -> MatchList:
        ...

    def __getitem__(self, i: int | slice):
        if isinstance(i, slice):
            return MatchList(self.root[i])
        match = self.root[i]
        assert isinstance(match, Match)
        return match

    def __iter__(self):
        return iter(self.root)

    def __str__(self):
        return " ".join(str(i) for i in self.root)

    @property
    def query(self):
        return "".join(x.query for x in iter(self))

    @property
    def state(self):
        return "".join(x.state for x in iter(self))

    @property
    def codon(self):
        return "".join(x.codon for x in iter(self))

    @property
    def amino(self):
        return "".join(x.amino for x in iter(self))


class MatchListInterval(PyInterval):
    ...


class MatchListIntervalBuilder:
    def __init__(self, match_list: MatchList):
        self._amino_map = [i for i, x in enumerate(match_list) if len(x.amino) > 0]

    def make_from_amino_interval(
        self, amino_interval: AminoInterval
    ) -> MatchListInterval:
        i = amino_interval
        x = self._amino_map[i.slice]
        return MatchListInterval(start=x[0], stop=x[-1] + 1)


class LazyMatchList(BaseModel):
    raw: str
    model_config = ConfigDict(frozen=True)

    @lru_cache(maxsize=1)
    def evaluate(self):
        return MatchList.from_string(self.raw)

    def __len__(self):
        return len(self.evaluate())

    def __getitem__(self, i):
        return self.evaluate()[i]

    def __iter__(self):
        return iter(self.evaluate())

    def __str__(self):
        return str(self.evaluate())

    def __repr__(self):
        return repr(self.evaluate())

    @property
    def query(self):
        return self.evaluate().query

    @property
    def state(self):
        return self.evaluate().state

    @property
    def codon(self):
        return self.evaluate().codon

    @property
    def amino(self):
        return self.evaluate().amino


def ifind(x: str, delim: str):
    start = 0
    end = x.find(delim, start)
    while end != -1:
        yield (start, end)
        start = end + 1
        end = x.find(delim, start)
    yield (start, len(x))
