import os
from pathlib import Path

import pytest

from deciphon_snap.amino import make_amino_interval
from deciphon_snap.hit import HitList
from deciphon_snap.match import MatchListIntervalBuilder
from deciphon_snap.read_snap import read_snap


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

    assert prod.query[:4] == "ATGC"
    assert prod.query[-4:] == "CTAA"

    assert prod.codon[:4] == "ATGC"
    assert prod.codon[-4:] == "ACTA"

    assert prod.amino[:4] == "MRDN"
    assert prod.amino[-4:] == "IKKL"

    assert len(prod.amino) == prod.h3result.domtbl[0].query.length

    hits = HitList.make(prod.match_list)
    domtbl = prod.h3result.domtbl
    assert len(hits) == 1
    assert len(domtbl) == 1

    matchs = prod.match_list[hits[0].match_list_interval.slice]
    assert matchs.amino[:4] == "FIYG"
    assert matchs.amino[-4:] == "GHKQ"

    mb = MatchListIntervalBuilder(prod.match_list)
    ai = make_amino_interval(domtbl[0].ali_coord)
    amino = prod.amino[ai.slice]
    sl = mb.make_from_amino_interval(ai).slice

    assert amino[:4] == "FIYG"
    assert amino[-4:] == "EISG"
    assert prod.match_list[sl].amino[:4] == amino[:4]
    assert prod.match_list[sl].amino[-4:] == amino[-4:]

    assert matchs.codon[:12] == "TTTATTTACGGT"
    assert matchs.codon[-12:] == "GGACATAAACAA"
    assert prod.match_list[sl].codon[:12] == matchs.codon[:12]
    assert prod.match_list[sl].codon[:-42][-12:] == matchs.codon[-12:]

    # assert matchs.codon[:12] == "TTTATTTACGGT"
    # assert matchs.codon[-12:] == "GGACATAAACAA"
