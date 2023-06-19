from __future__ import annotations

from dataclasses import dataclass
from typing import Optional, Iterator

from deciphon_core.cffi import ffi, lib
from deciphon_core.seq import Seq

__all__ = ["CSeq", "CSeqIter"]


@dataclass
class CSeq:
    id: int
    name: bytes
    data: bytes
    ctx: CSeqIter

    def __init__(self, seq: Seq, ctx: CSeqIter):
        self.id = seq.id
        self.name = seq.name.encode()
        self.data = seq.data.encode()
        self.ctx = ctx


class CSeqIter:
    def __init__(self, iter: Iterator[Seq]):
        self._cself = ffi.new_handle(self)
        self._iter = iter
        self._curr_seq: Optional[CSeq] = None

    def __enter__(self):
        return self

    def __exit__(self, *_):
        self._curr_seq = None

    def __next__(self) -> CSeq:
        # Keep the current returned sequence alive during
        # the lifetime of `self`.
        self._curr_seq = CSeq(next(self._iter), self)
        return self._curr_seq

    def __iter__(self):
        return self

    @property
    def c_callback(self):
        return lib.next_seq_callb

    @property
    def c_self(self):
        return self._cself


@ffi.def_extern()
def next_seq_callb(cseq, cself):
    iter: CSeqIter = ffi.from_handle(cself)
    try:
        seq: CSeq = next(iter)
        lib.dcp_seq_setup(cseq, seq.id, seq.name, seq.data)
    except Exception:
        return False
    return True
