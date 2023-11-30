from deciphon_snap.hit import HitList
from deciphon_snap.match import LazyMatchList, Match
from deciphon_snap.query_interval import QueryIntervalBuilder


def test_hits():
    query = "ATGAGCGTGAAAACCCTCTATCCGAAGAAACCGCAGAA"
    match_list = LazyMatchList(
        raw=(
            ",S,,;"
            "ATG,N,ATG,M;"
            "AGC,N,AGC,S;"
            ",B,,;"
            "GTG,M1,GTT,V;"
            "AAA,I2,AAA,K;"
            "ACC,M3,ACC,T;"
            ",D4,,;"
            ",D5,,;"
            ",E,,;"
            "CTC,J,CTC,L;"
            "TAT,J,TAT,Y;"
            "CCG,J,CCG,P;"
            "AAG,J,AAG,K;"
            ",B,,;"
            "AAA,M258,AAA,K;"
            "CCG,M259,CCG,P;"
            ",E,,;"
            "CAG,C,CAG,Q;"
            "AA,C,AAA,K;"
            ",T,,"
        )
    ).evaluate()
    hits = HitList.make(match_list)
    assert len(hits) == 2
    assert hits[0].id == 0
    assert hits[1].id == 1

    qibuilder = QueryIntervalBuilder(match_list)

    assert query[qibuilder.make(hits[0].match_list_interval).slice] == "GTGAAAACC"
    assert query[qibuilder.make(hits[1].match_list_interval).slice] == "AAACCG"

    x = match_list[hits[0].match_list_interval.slice]
    assert len(x) == 5
    assert str(x[0]) == str(Match.from_string("GTG,M1,GTT,V"))
    assert str(x[1]) == str(Match.from_string("AAA,I2,AAA,K"))
    assert str(x[2]) == str(Match.from_string("ACC,M3,ACC,T"))
    assert str(x[3]) == str(Match.from_string(",D4,,"))
    assert str(x[4]) == str(Match.from_string(",D5,,"))

    x = match_list[hits[1].match_list_interval.slice]
    assert len(x) == 2
    assert str(x[0]) == str(Match.from_string("AAA,M258,AAA,K"))
    assert str(x[1]) == str(Match.from_string("CCG,M259,CCG,P"))

    assert match_list[hits[0].match_list_interval.slice].query == "GTGAAAACC"
    assert match_list[hits[1].match_list_interval.slice].query == "AAACCG"

    assert match_list[hits[0].match_list_interval.slice].state == "M1I2M3D4D5"
    assert match_list[hits[1].match_list_interval.slice].state == "M258M259"

    assert match_list[hits[0].match_list_interval.slice].codon == "GTTAAAACC"
    assert match_list[hits[1].match_list_interval.slice].codon == "AAACCG"

    assert match_list[hits[0].match_list_interval.slice].amino == "VKT"
    assert match_list[hits[1].match_list_interval.slice].amino == "KP"
