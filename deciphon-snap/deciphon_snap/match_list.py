from typing import List

from pydantic import BaseModel

from deciphon_snap.match import Match

__all__ = ["MatchList"]


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
