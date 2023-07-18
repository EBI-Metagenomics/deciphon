from __future__ import annotations

import dataclasses
from collections.abc import Iterable
from typing import Any, Iterator

from deciphon_api.path_view import PathView

__all__ = ["HMMPath", "HMMStep", "HMMPathInterval"]


@dataclasses.dataclass
class HMMStep:
    query: str
    state: str
    codon: str
    amino: str
    query_index: int
    amino_index: int

    def has_query(self, level: int):
        return len(self.query) > level

    def has_codon(self):
        assert len(self.codon) == 0 or len(self.codon) == 3
        return len(self.codon) > 0

    def has_amino(self):
        return len(self.amino) > 0

    @property
    def core(self) -> bool:
        state = self.state.startswith
        return state("M") or state("I") or state("D")

    @property
    def mute(self) -> bool:
        return len(self.amino) == 0

    @property
    def state_position(self) -> int:
        if self.core:
            return int(self.state[1:]) - 0
        # TODO: warn the user?
        return -1

    def replace(self, changes: Any):
        return dataclasses.replace(self, **changes)


class HMMPathInterval(PathView):
    def __init__(self, path: HMMPath, start: int, end: int, hit: bool):
        self._start = start
        self._end = end
        self._hit = hit
        self._steps = [x for i, x in enumerate(path) if start <= i and i < end]

    @property
    def hit(self):
        return self._hit

    def __len__(self) -> int:
        return self._end - self._start

    def __getitem__(self, idx: int) -> HMMStep:
        return self._steps[idx]

    def __iter__(self) -> Iterator[HMMStep]:
        return iter(self._steps)


class HMMPath(PathView):
    def __init__(self, steps: Iterable[HMMStep]):
        self._steps = list(steps)

    @classmethod
    def make(cls, data: str):
        y = data.split(";")
        steps = [HMMStep(*m.split(","), query_index=0, amino_index=0) for m in y]
        qidx = [0]
        aidx = [0]
        for x in steps[:-1]:
            qidx += [len(x.query) + qidx[-1]]
            aidx += [len(x.amino) + aidx[-1]]
        return cls(
            [
                x.replace({"query_index": i, "amino_index": j})
                for x, i, j in zip(steps, qidx, aidx)
            ]
        )

    @property
    def intervals(self):
        return list(self._make_intervals())

    def _make_intervals(self):
        last = 0
        hit = False

        for i, x in enumerate(self._steps):
            if not hit and x.core:
                yield HMMPathInterval(self, last, i, hit)
                last = i
                hit = True
            elif hit and not x.core:
                yield HMMPathInterval(self, last, i, hit)
                last = i
                hit = False

        yield HMMPathInterval(self, last, len(self._steps), False)

    def __len__(self):
        return len(self._steps)

    def __getitem__(self, idx: int):
        return self._steps[idx]

    def __iter__(self):
        return iter(self._steps)
