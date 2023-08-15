from __future__ import annotations

import os
from tempfile import NamedTemporaryFile

from h3result.h3result import H3Result as H3ResultRaw
from hmmer_tables.domtbl import read_domtbl, DomTBL
from hmmer_tables.tbl import read_tbl, TBL
from pydantic import BaseModel, ConfigDict

__all__ = ["H3Result"]


class H3Result(BaseModel):
    raw: H3ResultRaw
    model_config = ConfigDict(arbitrary_types_allowed=True)

    @property
    def targets(self) -> str:
        tmp = TempFile()
        with tmp:
            self.raw.print_targets(tmp.fileno())
        return tmp.content

    @property
    def domains(self) -> str:
        tmp = TempFile()
        with tmp:
            self.raw.print_domains(tmp.fileno())
        return tmp.content

    @property
    def targets_table(self) -> str:
        tmp = TempFile()
        with tmp:
            self.raw.print_targets_table(tmp.fileno())
        return tmp.content

    @property
    def domains_table(self) -> str:
        tmp = TempFile()
        with tmp:
            self.raw.print_domains_table(tmp.fileno())
        return tmp.content

    @property
    def tbl(self) -> TBL:
        return read_tbl(stream=self.targets_table.split("\n"))

    @property
    def domtbl(self) -> DomTBL:
        return read_domtbl(stream=self.domains_table.split("\n"))

    def __str__(self):
        items = []
        for x in self.domtbl:
            e_value = x.full_sequence.e_value
            start = x.ali_coord.start
            stop = x.ali_coord.stop
            items.append(f"({e_value},{start}..{stop})")
        return "[{}]".format(",".join(items))

    def __repr__(self):
        return self.__str__()


class TempFile:
    def __init__(self):
        self._content = None
        self._stream = None

    def fileno(self):
        assert self._stream is not None
        return self._stream.fileno()

    @property
    def content(self):
        assert isinstance(self._content, str)
        return self._content

    def __enter__(self):
        self._stream = NamedTemporaryFile(delete=False)

    def __exit__(self, *args, **kwargs):
        del args
        del kwargs
        assert self._stream is not None
        self._stream.close()
        self._content = open(self._stream.name).read()
        os.unlink(self._stream.name)
