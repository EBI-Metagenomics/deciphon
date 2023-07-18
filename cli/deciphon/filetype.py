from enum import Enum, auto
from pathlib import Path

__all__ = ["Filetype"]


class Filetype(Enum):
    JSON = auto()
    FASTA = auto()

    @staticmethod
    def guess(file: Path):
        for row in open(file, "r"):
            if row.startswith(r"["):
                return Filetype.JSON
            if row.startswith(r">"):
                return Filetype.FASTA
        raise RuntimeError("Could not guess the file type.")
