from pathlib import Path

from hmmer_tables.query import read_query

from deciphon_snap.amino import make_amino_interval
from deciphon_snap.hit import HitList
from deciphon_snap.match import MatchListIntervalBuilder
from deciphon_snap.read_snap import read_snap


def test_same_amino(files_path: Path):
    prod = next(iter(read_snap(files_path / "consensus.dcs").products))
    domtbl = next(iter(prod.hmmer.domtbl))
    assert len(prod.amino) == domtbl.query.length
    query = read_query(stream=prod.hmmer.domains.split("\n"))
    assert len(query.domains) == 1
    assert query.domains[0].aligns[0].align.query_cs.upper() == prod.amino
    i = query.domains[0].aligns[0].align.query_interval
    assert i.stop - i.start + 1 == len(prod.amino)


def test_equal_hits(files_path: Path):
    for prod in read_snap(files_path / "consensus.dcs").products:
        hits = HitList.make(prod.match_list.evaluate())
        assert len(hits) == len(prod.hmmer.domtbl)
        for hit, domtbl in zip(hits, prod.hmmer.domtbl):
            x = hit.match_list_interval
            t = MatchListIntervalBuilder(prod.match_list.evaluate())
            y = t.make_from_amino_interval(make_amino_interval(domtbl.ali_coord))
            assert prod.match_list[x.slice].amino == prod.match_list[y.slice].amino
            assert prod.match_list[x.slice].query == prod.match_list[y.slice].query
            assert prod.match_list[x.slice].codon == prod.match_list[y.slice].codon
