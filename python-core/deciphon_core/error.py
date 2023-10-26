from deciphon_core.cffi import ffi, lib

__all__ = ["DeciphonError"]


class DeciphonError(RuntimeError):
    def __init__(self, errno: int):
        msg = ffi.string(lib.string_error(errno)).decode()
        super().__init__(f"deciphon_core error: {msg}")
