from __future__ import annotations

from io import StringIO
from typing import List, overload

from fasta_reader.writer import Writer
from pydantic import BaseModel, RootModel

__all__ = ["FASTAItem", "FASTAList"]


class FASTAItem(BaseModel):
    defline: str
    sequence: str


class FASTAList(RootModel):
    root: List[FASTAItem]

    def __len__(self):
        return len(self.root)

    @overload
    def __getitem__(self, i: int) -> FASTAItem:
        ...

    @overload
    def __getitem__(self, i: slice) -> FASTAList:
        ...

    def __getitem__(self, i: int | slice):
        if isinstance(i, slice):
            return FASTAList.model_validate(self.root[i])
        prod = self.root[i]
        assert isinstance(prod, FASTAItem)
        return prod

    def __iter__(self):
        return iter(self.root)

    def format(self, ncols: int = 60):
        t = StringIO()
        w = Writer(t, ncols=ncols)
        for x in self.root:
            w.write_item(x.defline, x.sequence)
        return t.getvalue()
