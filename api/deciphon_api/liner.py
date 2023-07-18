from __future__ import annotations

from io import StringIO, TextIOBase
from pathlib import Path
from typing import Iterator

__all__ = ["mkliner", "Liner"]


class Liner:
    def __init__(self, it: Iterator[str]):
        self._iter = it

    def __iter__(self):
        return Liner(iter(self._iter))

    def __next__(self):
        return next(self._iter)


def mkliner_data(obj: TextIOBase | str):
    if isinstance(obj, TextIOBase):
        return Liner(iter(obj))
    elif isinstance(obj, str):
        return Liner(iter(StringIO(obj)))
    else:
        assert False


def mkliner_path(path: Path | str):
    return Liner(iter(open(path, "r")))


def mkliner(
    data: TextIOBase | str | None = None, path: Path | str | None = None
) -> Liner:
    if data:
        return mkliner_data(data)
    elif path:
        return mkliner_path(path)
    raise ValueError("`data` and `path` cannot be both `None`")
