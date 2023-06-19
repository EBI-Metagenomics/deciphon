import dataclasses
from collections.abc import Iterable

from deciphon_api.coords import Coord, Interval, Point

__all__ = ["Viewport", "Pixel", "PixelList"]


@dataclasses.dataclass
class Pixel:
    point: Point
    char: str


class PixelList:
    def __init__(self, pixels: Iterable[Pixel]):
        self._pixels = list(pixels)

    def __getitem__(self, idx) -> Pixel:
        return self._pixels[idx]

    def __str__(self):
        return ", ".join(str(x) for x in self)


class Viewport:
    def __init__(self, coord: Coord, padding=" ", mask=None):
        self.coord = coord
        assert len(padding) == 1
        self._padding = padding
        if mask is None:
            self._mask = coord.as_interval()
        else:
            self._mask = mask

    def cut(self, interval: Interval):
        interval = interval.project(self.coord)
        return Viewport(interval.as_coord(), self._padding)

    def mask(self, interval: Interval):
        return Viewport(self.coord, self._padding, interval)

    def display(self, pixels: Iterable[Pixel]) -> str:
        arr = [self._padding] * self.coord.length
        i = self.coord.as_interval()
        j = self._mask
        for x in pixels:
            if i.has(x.point) and j.has(x.point):
                arr[x.point.project(self.coord).pos] = x.char[0]
        return "".join(arr)
