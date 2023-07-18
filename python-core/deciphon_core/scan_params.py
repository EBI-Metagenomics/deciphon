from pydantic import BaseModel

from deciphon_core.cffi import ffi

__all__ = ["ScanParams"]


class ScanParams(BaseModel):
    num_threads: int = 1
    lrt_threshold: float = 0.0
    multi_hits: bool = True
    hmmer3_compat: bool = False

    @property
    def cparams(self):
        params = ffi.new(
            "struct dcp_scan_params*",
            {
                "num_threads": self.num_threads,
                "lrt_threshold": self.lrt_threshold,
                "multi_hits": self.multi_hits,
                "hmmer3_compat": self.hmmer3_compat,
            },
        )
        if params == ffi.NULL:
            raise MemoryError()
        return params[0]
