from __future__ import annotations

import dataclasses

from deciphon_api.coords import Coord, Interval, Point
from deciphon_api.hmm_path import HMMPath, HMMPathInterval, HMMStep
from deciphon_api.hmmer_path import HMMERPath, HMMERStep
from deciphon_api.render import Pixel
from deciphon_api.right_join import RightJoin

__all__ = ["Align", "AlignSlice", "Pos", "HMMPos", "HMMERPos"]


class Checkpoint:
    def __init__(self, hmm_steps: list[HMMStep], hmmer_steps: list[HMMERStep]):
        self._hmm_steps = hmm_steps
        self._hmmer_steps = hmmer_steps

    def __call__(self, i: int, j: int) -> bool:
        x = self._hmm_steps
        y = self._hmmer_steps
        return not x[i].mute and y[j].match == "-"


def make_rjoins(hits: list[HMMPathInterval], hmmers: list[HMMERPath]):
    for hit, hmmer in zip(hits, hmmers):
        x = [i for i in hit]
        y = [i for i in hmmer]
        yield RightJoin(len(x), len(y), Checkpoint(x, y))


@dataclasses.dataclass
class Pos:
    point: Point
    hmm: HMMStep | None = None
    hmmer: HMMERStep | None = None


@dataclasses.dataclass
class HMMPos:
    point: Point
    hmm: HMMStep

    def amino(self):
        return Pixel(self.point, self.hmm.amino) if self.hmm.has_amino() else None

    def query(self, level: int):
        q = self.hmm.query
        return Pixel(self.point, q[level]) if self.hmm.has_query(level) else None


@dataclasses.dataclass
class HMMERPos:
    point: Point
    hmmer: HMMERStep

    def hmm_cs(self):
        return Pixel(self.point, self.hmmer.hmm_cs)

    def query_cs(self):
        return Pixel(self.point, self.hmmer.query_cs)

    def query(self):
        return Pixel(self.point, self.hmmer.query)

    def match(self):
        return Pixel(self.point, self.hmmer.match)

    def score(self):
        return Pixel(self.point, self.hmmer.score)


class AlignSlice:
    def __init__(self, positions: list[Pos], hit: bool):
        self._positions = positions
        self._hit: bool = hit

    @property
    def hit(self):
        return self._hit

    @property
    def interval(self):
        start = min([x.point.pos for x in self._positions])
        end = max([x.point.pos for x in self._positions]) + 1
        return Interval(self._positions[0].point.coord, start, end)

    # inclusive on both sizes, one-based numbering
    @property
    def query_bounds(self) -> tuple[int, int]:
        indices = [x.hmm.query_index for x in self._positions if x.hmm]
        if len(indices) == 0:
            # TODO: tell the user this is an error?
            return (0, 0)
        return (indices[0] + 1, indices[-1] + 1)

    # inclusive on both sizes, one-based numbering
    @property
    def state_bounds(self) -> tuple[int, int]:
        pos = [x.hmm.state_position for x in self._positions if x.hmm]
        if len(pos) == 0:
            # TODO: tell the user this is an error?
            return (0, 0)
        return (pos[0], pos[-1])

    # inclusive on both sizes, one-based numbering
    @property
    def amino_bounds(self) -> tuple[int, int]:
        indices = [x.hmm.amino_index for x in self._positions if x.hmm]
        if len(indices) == 0:
            # TODO: tell the user this is an error?
            return (0, 0)
        return (indices[0] + 1, indices[-1] + 1)

    @property
    def hmm(self):
        return [HMMPos(x.point, x.hmm) for x in self._positions if x.hmm]

    @property
    def hmmer(self):
        return [HMMERPos(x.point, x.hmmer) for x in self._positions if x.hmmer]

    def cut(self, interval: Interval):
        pos = [x for x in self._positions if interval.has(x.point)]
        return AlignSlice(pos, self.hit)


def make_slices(points, hmm: HMMPath, hmmers: list[HMMERPath]) -> list[AlignSlice]:
    intervals = []

    hits = [i for i in hmm.intervals if i.hit]
    assert len(hits) == len(hmmers)

    j = iter(hmmers)
    r = iter(list(make_rjoins(hits, hmmers)))
    for i in hmm.intervals:
        if not i.hit:
            intervals += [AlignSlice([Pos(next(points), x, None) for x in i], False)]
            continue

        a = iter(i)
        b = iter(next(j))
        c = next(r)
        intervals += [
            AlignSlice(
                [
                    Pos(next(points), next(a) if li else None, next(b) if ri else None)
                    for li, ri in zip(c.left, c.right)
                ],
                True,
            )
        ]

    return intervals


class Align:
    def __init__(self, hmm: HMMPath, headers: list[str], hmmers: list[HMMERPath]):
        self.headers = headers
        self._coord = Coord()
        points = self._coord.as_interval().points
        self._slices: list[AlignSlice] = make_slices(points, hmm, hmmers)

    @property
    def coord(self):
        return self._coord

    @property
    def hits(self) -> list[AlignSlice]:
        return [x for x in self._slices if x.hit]
