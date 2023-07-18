from io import StringIO

from BCBio import GFF
from Bio.SeqRecord import SeqRecord

__all__ = ["GFFWriter"]


class GFFWriter:
    def __init__(self):
        self._records: list[SeqRecord] = []

    def add(self, record: SeqRecord):
        self._records.append(record)

    def dumps(self) -> str:
        io = StringIO()
        GFF.write(sorted(self._records, key=lambda x: x.id), io, False)
        io.seek(0)
        return io.read()
