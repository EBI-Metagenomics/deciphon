from deciphon_core.cffi import ffi, lib
from deciphon_core.error import DeciphonError

__all__ = ["Params"]


class Params:
    def __init__(self, num_threads: int, multi_hits: bool, hmmer3_compat: bool):
        self._cptr = ffi.new("struct params *")
        if self._cptr == ffi.NULL:
            raise MemoryError()

        if rc := lib.params_setup(self._cptr, num_threads, multi_hits, hmmer3_compat):
            raise DeciphonError(rc)

    @property
    def cparams(self):
        return self._cptr[0]

    def __del__(self):
        if getattr(self, "_cparams", ffi.NULL) != ffi.NULL:
            lib.press_del(self._cptr)
