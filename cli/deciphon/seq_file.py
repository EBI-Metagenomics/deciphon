from __future__ import annotations

from collections.abc import Generator
from pathlib import Path
from typing import Optional, TextIO

import ijson
from deciphon_core.seq import Seq
from fasta_reader.reader import Reader as FASTAReader
from pydantic import BaseModel, FilePath

from deciphon.filetype import Filetype

__all__ = ["SeqFile"]


class SeqFile(BaseModel):
    path: FilePath

    def __enter__(self):
        self._type = Filetype.guess(Path(self.path))
        self._stream: Optional[TextIO] = None
        self._iter: Optional[Generator[Seq, None, None]] = None

        self._stream = open(self.path, "r")
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
