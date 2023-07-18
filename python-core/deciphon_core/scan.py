from __future__ import annotations

import shutil
from typing import Iterator

from deciphon_core.cffi import ffi, lib
from deciphon_core.cseq import CSeqIter
from deciphon_core.error import DeciphonError
from deciphon_core.hmmfile import HMMFile
from deciphon_core.seq import Seq
from deciphon_core.snapfile import NewSnapFile

__all__ = ["Scan"]


class Scan:
    def __init__(self, hmm: HMMFile, seqit: Iterator[Seq], snap: NewSnapFile):
        self._cscan = ffi.NULL
        self._hmm = hmm
        self._db = hmm.dbfile
        self._seqit = CSeqIter(seqit)
        self._snap = snap
        self._nthreads = 1
        self._port = 51371
        self._lrt_threshold = 10.0
        self._multi_hits = True
        self._hmmer3_compat = False

    @property
    def nthreads(self):
        return self._nthreads

    @nthreads.setter
    def nthreads(self, x: int):
        self._nthreads = x

    @property
    def port(self):
        return self._port

    @port.setter
    def port(self, x: int):
        self._port = x

    @property
    def lrt_threshold(self):
        return self._lrt_threshold

    @lrt_threshold.setter
    def lrt_threshold(self, x: float):
        self._lrt_threshold = x

    @property
    def multi_hits(self):
        return self._multi_hits

    @multi_hits.setter
    def multi_hits(self, x: bool):
        self._multi_hits = x

    @property
    def hmmer3_compat(self):
        return self._hmmer3_compat

    @hmmer3_compat.setter
    def hmmer3_compat(self, x: bool):
        self._hmmer3_compat = x

    def run(self):
        basedir = str(self._snap.basedir)
        if rc := lib.dcp_scan_run(self._cscan, basedir.encode()):
            raise DeciphonError(rc)

        archive = shutil.make_archive(basedir, "zip", base_dir=basedir)
        shutil.move(archive, self._snap.path)
        shutil.rmtree(self._snap.basedir)

    def open(self):
        self._cscan = lib.dcp_scan_new(self._port)
        if self._cscan == ffi.NULL:
            raise MemoryError()

        if rc := lib.dcp_scan_set_nthreads(self._cscan, self._nthreads):
            raise DeciphonError(rc)

        lib.dcp_scan_set_lrt_threshold(self._cscan, self._lrt_threshold)
        lib.dcp_scan_set_multi_hits(self._cscan, self._multi_hits)
        lib.dcp_scan_set_hmmer3_compat(self._cscan, self._hmmer3_compat)

        if rc := lib.dcp_scan_set_db_file(self._cscan, bytes(self._db.path)):
            raise DeciphonError(rc)

        it = self._seqit
        lib.dcp_scan_set_seq_iter(self._cscan, it.c_callback, it.c_self)

    def close(self):
        pass

    def __enter__(self):
        self.open()
        return self

    def __exit__(self, *_):
        self.close()

    def __del__(self):
        if getattr(self, "_cscan", ffi.NULL) != ffi.NULL:
            lib.dcp_scan_del(self._cscan)
