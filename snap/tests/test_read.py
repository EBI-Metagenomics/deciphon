from pathlib import Path


from deciphon_snap.read_snap import read_snap


def test_read_snap(files_path: Path):
    snap_file = read_snap(files_path / "example.dcs")
    assert len(snap_file.products) == 403
    prod = snap_file.products[0]
    assert prod.id == 0
    assert len(prod.match_list) == 160
    assert str(prod.match_list[0]) == "(∅,B,∅,∅)"

    assert prod.query[:4] == "CTCA"
    assert prod.query[-4:] == "TTCT"

    assert prod.codon[:4] == "CTCA"
    assert prod.codon[-4:] == "TTCT"

    assert prod.amino[:4] == "LKDI"
    assert prod.amino[-4:] == "EPTS"
