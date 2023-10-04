from subprocess import check_output

from index import Index

__all__ = ["SequenceIndex"]


class SequenceIndex(Index):
    def __init__(self, fasta: str):
        cmd = "grep '>' " + fasta + " | awk '{print $1}' | tr -d '>'"
        sequences = [
            x.strip().decode() for x in check_output(cmd, shell=True).strip().split()
        ]
        self._index = {
            x.replace("_cds_", "_any_"): i for i, x in enumerate(sorted(set(sequences)))
        }

    @property
    def count(self):
        return len(self._index)

    def index(self, sequence: str):
        return self._index[sequence]

    def keys(self):
        return self._index.keys()
