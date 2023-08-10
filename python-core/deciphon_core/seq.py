from dataclasses import dataclass

__all__ = ["Seq"]


@dataclass
class Seq:
    id: int
    name: str
    data: str
