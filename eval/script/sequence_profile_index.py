from profile_index import ProfileIndex
from sequence_index import SequenceIndex

__all__ = ["SequenceProfileIndex"]


class SequenceProfileIndex:
    def __init__(self, seq_idx: SequenceIndex, prof_idx: ProfileIndex):
        self._seq_idx = seq_idx
        self._prof_idx = prof_idx

    @property
    def count(self):
        return self._seq_idx.count * self._prof_idx.count

    def index(self, seqid: str, profile: str):
        n = self._prof_idx.count
        return self._seq_idx.index(seqid) * n + self._prof_idx.index(profile)
