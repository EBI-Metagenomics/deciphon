from profile_index import ProfileIndex

__all__ = ["ProfileNucleotideIndex"]


class ProfileNucleotideIndex:
    def __init__(self, prof_idx: ProfileIndex, genome_size: int):
        self._prof_idx = prof_idx
        self._genome_size = genome_size

    @property
    def count(self):
        return self._prof_idx.count * self._genome_size

    def index(self, profile: str, base_idx: int):
        n = self._genome_size
        return self._prof_idx.index(profile) * n + base_idx
