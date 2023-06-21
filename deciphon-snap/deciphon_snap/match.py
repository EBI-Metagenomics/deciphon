from __future__ import annotations

from typing import List

from pydantic import BaseModel

__all__ = ["Match", "MatchList", "LazyMatchList"]


class Match(BaseModel):
    query: str
    state: str
    codon: str
    amino: str

    @classmethod
    def from_string(cls, x: str):
        y = x.split(",", 3)
        return cls(query=y[0], state=y[1], codon=y[2], amino=y[3])

    def __str__(self):
        query = self.query if len(self.query) > 0 else "∅"
        state = self.state
        codon = self.codon if len(self.codon) > 0 else "∅"
        amino = self.amino if len(self.amino) > 0 else "∅"
        return f"({query},{state},{codon},{amino})"


class MatchList(BaseModel):
    __root__: List[Match]

    @classmethod
    def from_string(cls, x: str):
        return cls.parse_obj([Match.from_string(i) for i in x.split(";")])

    def __len__(self):
        return len(self.__root__)

    def __getitem__(self, i):
        return self.__root__[i]

    def __iter__(self):
        return iter(self.__root__)

    def __str__(self):
        return " ".join(str(i) for i in self.__root__)


class LazyMatchList(BaseModel):
    raw: str

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
