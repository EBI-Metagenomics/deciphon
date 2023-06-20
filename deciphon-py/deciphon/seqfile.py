from __future__ import annotations

from collections.abc import Generator
from pathlib import Path
from typing import Optional, TextIO

import ijson
from deciphon_core.seq import Seq
from fasta_reader.reader import Reader as FASTAReader

from deciphon.filetype import Filetype
from deciphon.path_like import PathLike

__all__ = ["SeqFile"]


class SeqFile:
    def __init__(self, file: PathLike):
        self._file = Path(file).absolute()

        if not self._file.exists():
            raise ValueError(f"`{self._file}` does not exist.")

        self._type = Filetype.guess(self._file)
        self._stream: Optional[TextIO] = None
        self._iter: Optional[Generator[Seq, None, None]] = None

    @property
    def path(self) -> Path:
        return self._file

    def __enter__(self):
        self._stream = open(self._file, "r")
        if self._type == Filetype.FASTA:
            self._iter = iter(fasta_items(self._stream))

        elif self._type == Filetype.JSON:
            self._iter = iter(json_items(self._stream))

        else:
            raise RuntimeError("Unknown file type.")
        return self

    def __exit__(self, *_):
        assert self._stream
        self._stream.close()

    def __next__(self):
        assert self._iter
        return next(self._iter)

    def __iter__(self):
        return self


def fasta_items(stream: TextIO):
    for i, x in enumerate(FASTAReader(stream)):
        yield Seq(i, name=x.defline, data=x.sequence)


def json_items(stream: TextIO):
    return (
        Seq(int(x["id"]), str(x["name"]), str(x["data"]))
        for x in ijson.items(stream, "item")
    )
