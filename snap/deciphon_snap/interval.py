from __future__ import annotations

from abc import ABC, abstractmethod

from pydantic import BaseModel, model_validator
from pydantic.types import Any, conint

__all__ = ["Interval", "PyInterval", "RInterval"]


class Interval(BaseModel, ABC):
    @property
    @abstractmethod
    def slice(self) -> slice:
        ...

    @property
    @abstractmethod
    def pyinterval(self) -> PyInterval:
        ...


class PyInterval(Interval):
    """
    Python interval.

    A Python interval is an 0-start, half-open interval. It means that:
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

    start: conint(ge=0)
    stop: conint(ge=0)

    @model_validator(mode="before")
    def pre_root(cls, values: dict[str, Any]) -> dict[str, Any]:
        assert values["start"] <= values["stop"]
        return values

    @property
    def rinterval(self) -> RInterval:
        return RInterval(start=self.start + 1, stop=self.stop)

    @property
    def pyinterval(self) -> PyInterval:
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

    An R interval is an 1-start, fully-closed. It means that:
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

    start: conint(gt=0)
    stop: conint(gt=0)

    @model_validator(mode="before")
    def pre_root(cls, values: dict[str, Any]) -> dict[str, Any]:
        assert values["start"] <= values["stop"]
        return values

    @property
    def pyinterval(self) -> PyInterval:
        return PyInterval(start=self.start - 1, stop=self.stop)

    @property
    def slice(self) -> slice:
        return self.pyinterval.slice

    def offset(self, offset: int) -> RInterval:
        return RInterval(start=self.start + offset, stop=self.stop + offset)

    def __str__(self):
        return repr(self)
