from deciphon_intervals import PyInterval
from hmmer_tables.domtbl import DomTBLCoord

__all__ = ["AminoInterval", "make_amino_interval"]


class AminoInterval(PyInterval):
    ...


def make_amino_interval(x: DomTBLCoord):
    if isinstance(x, DomTBLCoord):
        return AminoInterval(start=x.interval.start, stop=x.interval.stop)
    assert False
