from __future__ import annotations

from collections.abc import Iterable

__all__ = ["Coord", "Interval", "Point", "PointList"]


def common_coord_ancestor(a: Coord, b: Coord):
    if id(a) == id(b):
        return a
    if a.parent:
        return common_coord_ancestor(b, a.parent)
    elif b.parent:
        return common_coord_ancestor(b.parent, a)
    else:
        raise ValueError("Coordinates don't have common ancestor.")


class Coord:
    def __init__(self, length: int | None = None):
        if length:
            assert length >= 0
            self._length = length
        else:
            self._length = 2_147_483_648

    @property
    def offset(self):
        return 0

    @property
    def length(self):
        return self._length

    def as_interval(self):
        return Interval(self, 0, self.length)

    @property
    def parent(self):
        return None

    def make_interval(self, start: int, end: int) -> Interval:
        return Interval(self, start, end)

    def make_point(self, pos: int) -> Point:
        return Point(self, pos)

    def make_coord(self, start: int, end: int):
        return OffsetCoord(self, start, end - start)

    def __str__(self):
        return f"Coord(length={self.length})"


class OffsetCoord(Coord):
    def __init__(self, parent: Coord, offset: int, length: int):
        assert length >= 0
        assert parent.length >= offset + length
        self._parent = parent
        self._offset = offset
        self._length = length

    @property
    def offset(self):
        return self._offset

    @property
    def parent(self):
        return self._parent

    def __str__(self):
        return f"OffsetCoord(offset={self.offset}, length={self.length})"


class Interval:
    def __init__(self, coord: Coord, start: int, end: int):
        self.coord = coord
        self.start = start
        self.end = end

    @property
    def points(self):
        for i in range(self.start, self.end):
            yield Point(self.coord, i)

    @property
    def length(self):
        return self.end - self.start

    def as_coord(self):
        return self.coord.make_coord(self.start, self.end)

    def _project_onto_ancestor(self, ancestor: Coord) -> Interval:
        start = self.start
        end = self.end
        for x in coord_up(self.coord):
            if id(x) == id(ancestor):
                break
            start += x.offset
            end += x.offset
        return ancestor.make_interval(start, end)

    def _project_onto_descendant(self, descendant: Coord) -> Interval:
        start = self.start
        end = self.end
        coords = []
        for x in coord_up(descendant):
            coords.append(x)
            if id(x) == id(self.coord):
                break
        for x in reversed(coords):
            start -= x.offset
            end -= x.offset
        return descendant.make_interval(start, end)

    def project(self, coord: Coord) -> Interval:
        ancestor = common_coord_ancestor(self.coord, coord)
        interval = self._project_onto_ancestor(ancestor)
        return interval._project_onto_descendant(coord)

    def has(self, point: Point):
        coord = common_coord_ancestor(self.coord, point.coord)
        i = self.project(coord)
        p = point.project(coord)
        return i.start <= p.pos and p.pos < i.end

    def intervals(self, size: int | None = None):
        assert size
        for start in range(self.start, self.end, size):
            end = min(start + size, self.end)
            yield self.coord.make_interval(start, end)

    def __str__(self):
        return f"[{self.start}, {self.end})"


def coord_up(coord: Coord):
    x: Coord | None = coord
    while x:
        yield x
        x = x.parent


class Point:
    def __init__(self, coord: Coord, pos: int):
        assert pos < coord.length
        self.coord = coord
        self.pos = pos

    def _project_onto_ancestor(self, ancestor: Coord) -> Point:
        pos = self.pos
        for x in coord_up(self.coord):
            if id(x) == id(ancestor):
                break
            pos += x.offset
        return ancestor.make_point(pos)

    def _project_onto_descendant(self, descendant: Coord) -> Point:
        pos = self.pos
        coords = []
        for x in coord_up(descendant):
            coords.append(x)
            if id(x) == id(self.coord):
                break
        for x in reversed(coords):
            pos -= x.offset
        return descendant.make_point(pos)

    def project(self, coord: Coord) -> Point:
        ancestor = common_coord_ancestor(self.coord, coord)
        point = self._project_onto_ancestor(ancestor)
        return point._project_onto_descendant(coord)

    def __str__(self):
        return f"[{self.pos}]"


class PointList:
    def __init__(self, points: Iterable[Point]):
        self._points = list(points)

    def __getitem__(self, idx) -> Point:
        return self._points[idx]

    def __str__(self):
        return ", ".join(str(x) for x in self)
