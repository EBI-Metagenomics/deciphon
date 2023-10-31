from pathlib import Path


from deciphon_snap.read_snap import read_snap


def test_read_snap(files_path: Path):
    snap_file = read_snap(files_path / "example.dcs")
    assert len(snap_file.products) == 403
    prod = snap_file.products[0]
    assert prod.id == 0
    assert len(prod.match_list) == 263
    assert str(prod.match_list[0]) == "(∅,S,∅,∅)"

    assert prod.query[:4] == "ATGA"
    assert prod.query[-4:] == "GTAA"

    assert prod.codon[:4] == "ATGA"
    assert prod.codon[-4:] == "TAAA"

    assert prod.amino[:4] == "MSVE"
    assert prod.amino[-4:] == "GSGK"
