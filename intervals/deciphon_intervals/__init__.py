from __future__ import annotations

from abc import ABC, abstractmethod
from typing import Any

from pydantic import BaseModel, NonNegativeInt, PositiveInt, model_validator

__all__ = ["Interval", "PyInterval", "RInterval"]


class Interval(BaseModel, ABC):
    @property
    @abstractmethod
    def r(self) -> RInterval:
        ...

    @property
    @abstractmethod
    def py(self) -> PyInterval:
        ...

    @property
    @abstractmethod
    def slice(self) -> slice:
        ...


class PyInterval(Interval):
    """
    Python interval.

    A Python interval is an 0-start, half-open one. It means that:
    - The elements of a sequence have the coordinates `0, 1, 2, ...`.
    - An interval `PyInterval(start, stop)` is defined by the coordinates
      `start, start+1, ..., stop-2, stop-1`.

    Attributes
    ----------
    start
        Start of interval. Valid values are `0, 1, ..., stop`.
    stop
        End of interval. Valid values are `start, start+1, ...`.
    """

    start: NonNegativeInt
    stop: NonNegativeInt

    @model_validator(mode="before")
    def pre_root(cls, values: dict[str, Any]) -> dict[str, Any]:
        assert values["start"] <= values["stop"]
        return values

    @property
    def r(self) -> RInterval:
        return RInterval(start=self.start + 1, stop=self.stop)

    @property
    def py(self) -> PyInterval:
        return self

    @property
    def slice(self) -> slice:
        return slice(self.start, self.stop)

    def offset(self, offset: int) -> PyInterval:
        return PyInterval(start=self.start + offset, stop=self.stop + offset)

    def __str__(self):
        return repr(self)


class RInterval(Interval):
    """
    R interval.

    An R interval is an 1-start, fully-closed one. It means that:
    - The elements of a sequence have the coordinates `1, 2, 3, ...`.
    - An interval `RInterval(start, stop)` is defined by the coordinates
      `start, start+1, ..., stop-1, stop`.

    Attributes
    ----------
    start
        Start of interval. Valid values are `1, 2, ..., stop`.
    stop
        End of interval. Valid values are `start, start+1, ...`.
    """

    start: PositiveInt
    stop: PositiveInt

    @model_validator(mode="before")
    def pre_root(cls, values: dict[str, Any]) -> dict[str, Any]:
        assert values["start"] <= values["stop"]
        return values

    @property
    def r(self) -> RInterval:
        return self

    @property
    def py(self) -> PyInterval:
        return PyInterval(start=self.start - 1, stop=self.stop)

    @property
    def slice(self) -> slice:
        return self.py.slice

    def offset(self, offset: int) -> RInterval:
        return RInterval(start=self.start + offset, stop=self.stop + offset)

    def __str__(self):
        return repr(self)
