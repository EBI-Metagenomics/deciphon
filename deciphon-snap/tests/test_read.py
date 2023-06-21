import os
from pathlib import Path
from deciphon_snap.read_snap import read_snap

import pytest


@pytest.fixture()
def files_path() -> Path:
    current_dir = os.path.dirname(__file__)
    return Path(current_dir) / "files"


def test_read_snap(files_path: Path):
    snap_file = read_snap(files_path / "example.dcs")
    assert len(snap_file.products) == 470
    prod = snap_file.products[0]
    assert prod.id == 0
    assert len(prod.match_list) == 462
    assert str(prod.match_list[0]) == "(∅,S,∅,∅)"

    assert prod.match_list.query[:4] == "ATGC"
    assert prod.match_list.query[-4:] == "CTAA"

    assert prod.match_list.codon[:4] == "ATGC"
    assert prod.match_list.codon[-4:] == "ACTA"

    assert prod.match_list.amino[:4] == "MRDN"
    assert prod.match_list.amino[-4:] == "IKKL"
