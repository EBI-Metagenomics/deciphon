from deciphon_core.cffi import ffi, lib

__all__ = ["DeciphonError"]


class DeciphonError(RuntimeError):
    def __init__(self, errno: int):
        msg = ffi.string(lib.error_string(errno)).decode()
        super().__init__(msg)
