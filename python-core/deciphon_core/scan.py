from __future__ import annotations


from deciphon_core.cffi import ffi, lib
from deciphon_core.error import DeciphonError
from deciphon_core.params import Params
from deciphon_core.schema import DBFile, NewSnapFile
from deciphon_core.sequence import Sequence

__all__ = ["Scan"]


class Scan:
    def __init__(self, params: Params, dbfile: DBFile):
        self._cscan = lib.scan_new(params.cparams)
        if self._cscan == ffi.NULL:
            raise MemoryError()
        self._dbfile = dbfile

    def dial(self, port: int = 51371):
        if rc := lib.scan_dial(self._cscan, port):
            raise DeciphonError(rc)

    def open(self):
        if rc := lib.scan_open(self._cscan, bytes(self._dbfile.path)):
            raise DeciphonError(rc)

    def close(self):
        if rc := lib.scan_close(self._cscan):
            raise DeciphonError(rc)

    def add(self, sequence: Sequence):
        id = sequence.id
        name = sequence.name.encode()
        data = sequence.data.encode()
        if rc := lib.scan_add(self._cscan, id, name, data):
            raise DeciphonError(rc)

    def run(self, snap: NewSnapFile):
        if rc := lib.scan_run(self._cscan, str(snap.basename).encode()):
            raise DeciphonError(rc)

        snap.make_archive()

    def progress(self) -> int:
        return lib.scan_progress(self._cscan)

    def __enter__(self):
        self.open()
        return self

    def __exit__(self, *_):
        self.close()

    def __del__(self):
        if getattr(self, "_cscan", ffi.NULL) != ffi.NULL:
            lib.scan_del(self._cscan)
