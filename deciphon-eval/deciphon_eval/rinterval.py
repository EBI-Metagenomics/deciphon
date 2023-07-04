from __future__ import annotations

import re

import deciphon_snap.interval
import hmmer_tables.interval
import portion
from pydantic import BaseModel, model_validator
from pydantic.types import Any, conint

__all__ = ["RInterval"]


repr_regex: re.Pattern = re.compile(r"^RInterval\((\d+),(\d+)\)$")


class RInterval(BaseModel):
    start: conint(gt=0)
    stop: conint(gt=0)

    @model_validator(mode="before")
    def pre_root(cls, values: dict[str, Any]) -> dict[str, Any]:
        assert values["start"] <= values["stop"]
        return values

    @property
    def slice(self) -> slice:
        return slice(self.start - 1, self.stop)

    def offset(self, offset: int) -> RInterval:
        return RInterval(start=self.start + offset, stop=self.stop + offset)

    @classmethod
    def parse_obj(
        cls, x: hmmer_tables.interval.RInterval | deciphon_snap.interval.RInterval
    ):
        assert isinstance(x, hmmer_tables.interval.RInterval) or isinstance(
            x, deciphon_snap.interval.RInterval
        )
        return cls(start=x.start, stop=x.stop)

    @classmethod
    def from_string(cls, raw: str):
        match = repr_regex.match(raw)
        assert match is not None
        return cls(start=int(match.group(1)), stop=int(match.group(2)))

    def to_portion(self) -> portion.Interval:
        return portion.closed(self.start, self.stop)

    def __repr__(self):
        return f"RInterval({self.start},{self.stop})"

    def __str__(self):
        return f"RInterval({self.start},{self.stop})"

    def nuclt_to_amino(self):
        start = _nuclt_to_amino(self.start)
        stop = _nuclt_to_amino(self.stop - 2)
        return RInterval(start=start, stop=stop)

    def amino_to_nuclt(self):
        start = _amino_to_nuclt(self.start)
        stop = _amino_to_nuclt(self.stop)
        return RInterval(start=start, stop=stop + 2)

    def iterate(self):
        return range(self.start, self.stop + 1)


def _nuclt_to_amino(x: int):
    return (x - 1) // 3 + 1


def _amino_to_nuclt(x: int):
    return (x - 1) * 3 + 1


if __name__ == "__main__":
    x = RInterval(start=1, stop=3)
    for i in x.iterate():
        print(i)
