from __future__ import annotations

from pydantic import BaseModel, root_validator
from pydantic.types import conint

__all__ = ["PyInterval", "RInterval"]


class PyInterval(BaseModel):
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

    @root_validator()
    @classmethod
    def root_validator(cls, field_values):
        assert field_values["start"] <= field_values["stop"]
        return field_values

    @property
    def rinterval(self) -> RInterval:
        return RInterval(start=self.start + 1, stop=self.stop)

    @property
    def slice(self) -> slice:
        return slice(self.start, self.stop)

    def offset(self, offset: int) -> PyInterval:
        return PyInterval(start=self.start + offset, stop=self.stop + offset)

    def __str__(self):
        return repr(self)


class RInterval(BaseModel):
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

    @root_validator()
    @classmethod
    def root_validator(cls, field_values):
        assert field_values["start"] <= field_values["stop"]
        return field_values

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
