from __future__ import annotations

from pydantic import BaseModel

__all__ = ["Match"]


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
