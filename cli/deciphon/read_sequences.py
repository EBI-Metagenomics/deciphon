from __future__ import annotations

from pathlib import Path
from typing import TextIO

import ijson
from deciphon_core.sequence import Sequence
from fasta_reader.reader import Reader as FASTAReader
from pydantic import FilePath

from deciphon.filetype import Filetype

__all__ = ["read_sequences"]


def read_sequences(file: FilePath):
    type = Filetype.guess(Path(file))
    with open(file, "r") as stream:
        if type == Filetype.FASTA:
            return list(read_fasta(stream))
        elif type == Filetype.JSON:
            return list(read_json(stream))
        else:
            raise RuntimeError("Unknown file type.")


def read_fasta(stream: TextIO):
    for i, x in enumerate(FASTAReader(stream)):
        yield Sequence(i, name=x.defline, data=x.sequence)


def read_json(stream: TextIO):
    return (
        Sequence(int(x["id"]), str(x["name"]), str(x["data"]))
        for x in ijson.items(stream, "item")
    )
