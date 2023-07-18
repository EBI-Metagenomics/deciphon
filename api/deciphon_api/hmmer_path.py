from __future__ import annotations

import dataclasses
from collections.abc import Iterable

from deciphon_api.path_view import PathView

__all__ = ["HMMERPath", "HMMERStep"]


@dataclasses.dataclass
class HMMERStep:
    hmm_cs: str
    query_cs: str
    match: str
    query: str
    score: str


class HMMERPath(PathView):
    def __init__(self, steps: Iterable[HMMERStep]):
        self._steps = list(steps)

    def __len__(self):
        return len(self._steps)

    def __getitem__(self, idx: int):
        return self._steps[idx]

    def __iter__(self):
        return iter(self._steps)
