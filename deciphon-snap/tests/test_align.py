import os
from pathlib import Path

import pytest
from hmmer_tables.query import read_query

from deciphon_snap.amino import make_amino_interval
from deciphon_snap.hit import HitList
from deciphon_snap.match import MatchListIntervalBuilder
from deciphon_snap.read_snap import read_snap


@pytest.fixture()
def files_path() -> Path:
    current_dir = os.path.dirname(__file__)
    return Path(current_dir) / "files"


def test_similar_amino(files_path: Path):
    prods = read_snap(files_path / "example2.dcs").products
    assert len(prods) == 2
    prod = prods[0]
    assert len(prod.hmmer.domtbl) == 2

    domtbl = prod.hmmer.domtbl[1]
    assert domtbl.query.length == 338
    assert domtbl.ali_coord.start == 172
    assert domtbl.ali_coord.stop == 338

    query = read_query(stream=prod.hmmer.domains.split("\n"))
    assert len(query.domains[0].aligns) == 2
    align = query.domains[0].aligns[1].align
    assert align.query_interval.start == 172
    assert align.query_interval.stop == 338

    hits = HitList.make(prod.match_list)
    assert len(hits) == 2
    hit = hits[1]
    x = hit.match_list_interval
    t = MatchListIntervalBuilder(prod.match_list)
    y = t.make_from_amino_interval(make_amino_interval(domtbl.ali_coord))

    assert prod.match_list[x.slice][0].state == "M4"
    assert align.core_interval.start == 3

    assert prod.match_list[x.slice][-1].state == "M170"
    assert align.core_interval.stop == 170

    assert prod.match_list[x.slice].amino == prod.match_list[y.slice].amino[1:]
    assert prod.match_list[x.slice].amino == align.query.replace("-", "", 1)[1:]


def test_same_amino(files_path: Path):
    prod = next(iter(read_snap(files_path / "consensus.dcs").products))
    # prod = next(iter(read_snap(files_path / "example.dcs").products))
    domtbl = next(iter(prod.hmmer.domtbl))
    assert len(prod.amino) == domtbl.query.length
    query = read_query(stream=prod.hmmer.domains.split("\n"))
    assert len(query.domains) == 1
    assert query.domains[0].aligns[0].align.query_cs.upper() == prod.amino
    i = query.domains[0].aligns[0].align.query_interval
    assert i.stop - i.start + 1 == len(prod.amino)


def test_equal_hits(files_path: Path):
    for prod in read_snap(files_path / "consensus.dcs").products:
        hits = HitList.make(prod.match_list)
        assert len(hits) == len(prod.hmmer.domtbl)
        for hit, domtbl in zip(hits, prod.hmmer.domtbl):
            x = hit.match_list_interval
            t = MatchListIntervalBuilder(prod.match_list)
            y = t.make_from_amino_interval(make_amino_interval(domtbl.ali_coord))
            assert prod.match_list[x.slice].amino == prod.match_list[y.slice].amino
            assert prod.match_list[x.slice].query == prod.match_list[y.slice].query
            assert prod.match_list[x.slice].codon == prod.match_list[y.slice].codon


def test_align(files_path: Path):
    snap_file = read_snap(files_path / "example.dcs")
    assert len(snap_file.products) == 470
    prod = snap_file.products[0]
    assert prod.id == 0
    assert len(prod.match_list) == 462
    assert str(prod.match_list[0]) == "(∅,S,∅,∅)"

    assert len(prod.amino) == prod.hmmer.domtbl[0].query.length

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
