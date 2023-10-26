from deciphon_core.cffi import ffi, lib
from deciphon_core.error import DeciphonError
from deciphon_core.schema import Gencode, HMMFile

__all__ = ["PressContext"]


class PressContext:
    def __init__(self, hmm: HMMFile, gencode: Gencode, epsilon: float = 0.01):
        self._cpress = lib.press_new()
        self._hmm = hmm

        if self._cpress == ffi.NULL:
            raise MemoryError()

        if rc := lib.press_setup(self._cpress, gencode, epsilon):
            raise DeciphonError(rc)

    def open(self):
        hmmpath = bytes(self._hmm.path)
        dbpath = bytes(self._hmm.newdbfile.path)
        if rc := lib.press_open(self._cpress, hmmpath, dbpath):
            raise DeciphonError(rc)

    def close(self):
        if rc := lib.press_close(self._cpress):
            raise DeciphonError(rc)

    def end(self) -> bool:
        return lib.press_end(self._cpress)

    def next(self):
        if rc := lib.press_next(self._cpress):
            raise DeciphonError(rc)

    def __enter__(self):
        self.open()
        return self

    def __exit__(self, *_):
        self.close()

    @property
    def nproteins(self) -> int:
        return lib.press_nproteins(self._cpress)

    def __del__(self):
        if getattr(self, "_cpress", ffi.NULL) != ffi.NULL:
            lib.press_del(self._cpress)
