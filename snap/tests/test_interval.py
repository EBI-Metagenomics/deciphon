from deciphon_snap.interval import PyInterval, RInterval


def test_interval():
    x = PyInterval(start=1, stop=4)

    seq = "ABCDE"
    assert seq[x.slice] == "BCD"
    assert seq[x.rinterval.slice] == "BCD"
    assert seq[x.rinterval.pyinterval.slice] == "BCD"

    assert x.start == 1
    assert x.stop == 4
    assert x.rinterval.start == 2
    assert x.rinterval.stop == 4

    assert repr(x) == "PyInterval(start=1, stop=4)"
    assert repr(x.rinterval) == "RInterval(start=2, stop=4)"

    y = RInterval(start=2, stop=4)
    assert seq[x.slice] == seq[y.slice]
