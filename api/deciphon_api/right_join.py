from collections.abc import Callable, Iterable
from typing import Any, Iterator

__all__ = ["RightJoin"]


def expand(founds: Iterable[bool], items: Iterator[Any], padding):
    for i in founds:
        if i:
            yield next(items)
        else:
            yield padding


class RightJoin:
    def __init__(self, n: int, m: int, chkpoint: Callable[[int, int], bool]):
        self._a = []
        self._b = []
        i, j = 0, 0

        while i < n and j < m:
            if chkpoint(i, j):
                self._a.append(False)
                j += 1
            else:
                self._a.append(True)
                self._b.append(True)
                i += 1
                j += 1

        while i < n:
            self._a.append(True)
            self._b.append(False)
            i += 1

        while j < m:
            self._a.append(False)
            self._b.append(True)
            j += 1

        assert len(self._a) == len(self._b)

    def left_expand(self, items: Iterator[Any], padding):
        return expand(self._a, items, padding)

    def right_expand(self, items: Iterator[Any], padding):
        return expand(self._b, items, padding)

    @property
    def left(self) -> list[bool]:
        return self._a

    @property
    def right(self) -> list[bool]:
        return self._b

    @property
    def size(self) -> int:
        return len(self._a)
