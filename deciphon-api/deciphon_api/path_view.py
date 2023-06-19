from __future__ import annotations

from abc import ABC, abstractmethod
from typing import Generic, Iterator, TypeVar

__all__ = ["PathView"]

T = TypeVar("T")


class PathView(ABC, Generic[T]):
    @abstractmethod
    def __len__(self) -> int:
        ...

    @abstractmethod
    def __getitem__(self, idx: int) -> T:
        ...

    @abstractmethod
    def __iter__(self) -> Iterator[T]:
        ...
