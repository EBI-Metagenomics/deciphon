from __future__ import annotations

from deciphon_core.batch import Batch
from deciphon_core.cffi import ffi, lib
from deciphon_core.error import DeciphonError
from deciphon_core.schema import DBFile, NewSnapFile

__all__ = ["Scan"]


def check_exception(exception, exc_value, traceback):
    if traceback is not None:
        scan: Scan = ffi.from_handle(traceback.tb_frame.f_locals["userdata"])
        scan.interrupt()
        scan.interrupted = True


@ffi.def_extern(onerror=check_exception)
def callback(userdata):
    pass


class Scan:
    def __init__(
        self,
        dbfile: DBFile,
        port: int,
        num_threads: int,
        multi_hits: bool,
        hmmer3_compat: bool,
        cache: bool,
    ):
        self._cscan = lib.dcp_scan_new()
        if self._cscan == ffi.NULL:
            raise MemoryError()
        self._handle = ffi.new_handle(self)

        self.interrupted = False

        if rc := lib.dcp_scan_setup(
            self._cscan,
            bytes(dbfile.path),
            port,
            num_threads,
            multi_hits,
            hmmer3_compat,
            cache,
            lib.callback,
            self._handle,
        ):
            raise DeciphonError(rc)

    def run(self, snap: NewSnapFile, batch: Batch):
        self.interrupted = False
        if rc := lib.dcp_scan_run(
            self._cscan, batch.cdata, str(snap.basename).encode()
        ):
            raise DeciphonError(rc)

    def interrupt(self):
        lib.dcp_scan_interrupt(self._cscan)

    def progress(self) -> int:
        return lib.dcp_scan_progress(self._cscan)

    def free(self):
        if getattr(self, "_cscan", ffi.NULL) != ffi.NULL:
            lib.dcp_scan_del(self._cscan)
            self._cscan = ffi.NULL

    def __del__(self):
        self.free()

    def __enter__(self):
        return self

    def __exit__(self, *_):
        self.free()
