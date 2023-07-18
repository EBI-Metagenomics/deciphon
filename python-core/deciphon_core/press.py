from __future__ import annotations

from typing import Optional

from deciphon_core.cffi import ffi, lib
from deciphon_core.error import DeciphonError
from deciphon_core.hmmfile import HMMFile

__all__ = ["Press"]


class HMMPress:
    def __init__(self, press: Press):
        self._press: Optional[Press] = press

    def press(self):
        assert self._press
        if rc := lib.dcp_press_next(self._press.cpress):
            raise DeciphonError(rc)
        self._press = None


class Press:
    def __init__(self, hmm: HMMFile, codon_table: int = 1):
        self._cpress = ffi.NULL
        self._hmm = hmm
        self._db = hmm.newdbfile
        self._nproteins: int = -1
        self._idx = -1
        self._codon_table = codon_table

    @property
    def cpress(self):
        return self._cpress

    def open(self):
        self._cpress = lib.dcp_press_new()
        if self._cpress == ffi.NULL:
            raise MemoryError()

        hmmpath = bytes(self._hmm.path)
        dbpath = bytes(self._db.path)
        if rc := lib.dcp_press_open(self._cpress, self._codon_table, hmmpath, dbpath):
            raise DeciphonError(rc)

    def close(self):
        if rc := lib.dcp_press_close(self._cpress):
            raise DeciphonError(rc)

    def __enter__(self):
        self.open()
        return self

    def __exit__(self, *_):
        self.close()

    @property
    def nproteins(self) -> int:
        if self._nproteins == -1:
            self._nproteins: int = lib.dcp_press_nproteins(self._cpress)
        return self._nproteins

    def __len__(self):
        return self.nproteins

    def __iter__(self):
        return self

    def __next__(self):
        self._idx += 1
        if self._idx < self.nproteins:
            return HMMPress(self)
        raise StopIteration()

    def __del__(self):
        if getattr(self, "_cpress", ffi.NULL) != ffi.NULL:
            lib.dcp_press_del(self._cpress)
