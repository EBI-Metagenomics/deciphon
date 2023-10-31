from __future__ import annotations

from typing import List, overload

from pydantic import BaseModel, RootModel


__all__ = ["GFFItem", "GFFList"]


class GFFItem(BaseModel):
    seqid: str
    source: str
    type: str
    start: int
    end: int
    score: float
    strand: str
    phase: str
    attributes: str

    def format(self):
        score = f"{self.score:.2g}"
        return (
            f"{self.seqid}\t{self.source}\t{self.type}\t{self.start}\t{self.end}\t"
            + f"{score}\t{self.strand}\t{self.phase}\t{self.attributes}"
        )


class GFFList(RootModel):
    root: List[GFFItem]

    def __len__(self):
        return len(self.root)

    @overload
    def __getitem__(self, i: int) -> GFFItem:
        ...

    @overload
    def __getitem__(self, i: slice) -> GFFList:
        ...

    def __getitem__(self, i: int | slice):
        if isinstance(i, slice):
            return GFFList.model_validate(self.root[i])
        prod = self.root[i]
        assert isinstance(prod, GFFItem)
        return prod

    def __iter__(self):
        return iter(self.root)

    def format(self):
        gffs = [GFFItem.model_validate(x) for x in self.root]
        for i, x in enumerate(gffs):
            x.attributes = x.attributes + f";ID={i+1}"
        return "\n".join(["##gff-version 3"] + [x.format() for x in gffs]) + "\n"
