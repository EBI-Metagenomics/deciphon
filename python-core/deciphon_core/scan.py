from __future__ import annotations

from typing import Iterator

from deciphon_core.cffi import ffi, lib
from deciphon_core.cseq import CSeqIter
from deciphon_core.error import DeciphonError
from deciphon_core.scan_params import ScanParams
from deciphon_core.schema import DBFile, NewSnapFile
from deciphon_core.seq import Seq

__all__ = ["Scan"]


class Scan:
    def __init__(self):
        self._cscan = lib.dcp_scan_new()
        if self._cscan == ffi.NULL:
            raise MemoryError()

    def dial(self, port: int = 51371):
        if rc := lib.dcp_scan_dial(self._cscan, port):
            raise DeciphonError(rc)

    def setup(self, params: ScanParams):
        if rc := lib.dcp_scan_setup(self._cscan, params.cparams):
            raise DeciphonError(rc)

    def run(self, dbfile: DBFile, seqit: Iterator[Seq], snap: NewSnapFile):
        cscan = self._cscan
        db = bytes(dbfile.path)
        it = CSeqIter(seqit)
        basename = str(snap.basename).encode()
        if rc := lib.dcp_scan_run(cscan, db, it.c_callback, it.c_self, basename):
            raise DeciphonError(rc)

        snap.make_archive()

    def __del__(self):
        if getattr(self, "_cscan", ffi.NULL) != ffi.NULL:
            lib.dcp_scan_del(self._cscan)
