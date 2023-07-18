import dataclasses
from typing import List

from deciphon_api.models import Prod

__all__ = ["make_hits"]


@dataclasses.dataclass
class Match:
    state: str
    query: str
    codon: str
    amino: str

    def get(self, field: str):
        return dataclasses.asdict(self)[field]


@dataclasses.dataclass
class Hit:
    id: int
    name: str
    prod_id: int
    evalue: float
    matchs: List[Match] = dataclasses.field(default_factory=lambda: [])
    feature_start: int = 0
    feature_end: int = 0


def is_core_state(state: str):
    return state.startswith("M") or state.startswith("I") or state.startswith("D")


def make_hits(prod: Prod) -> List[Hit]:
    hits: List[Hit] = []

    hit_start = 0
    hit_end = 0
    offset = 0
    hit_start_found = False
    hit_end_found = False

    for query_match in prod.match.split(";"):
        query, state, codon, amino = query_match.split(",")

        if not hit_start_found and is_core_state(state):
            hit_start = offset
            hit_start_found = True
            evalue = prod.evalue
            name = prod.seq.name
            assert prod.id
            hits.append(Hit(len(hits) + 1, name, prod.id, evalue))

        if hit_start_found and not is_core_state(state):
            hit_end = offset + len(query)
            hit_end_found = True

        if hit_start_found and not hit_end_found:
            hits[-1].matchs.append(Match(state[0], query, codon, amino))

        if hit_end_found:
            hits[-1].feature_start = hit_start
            hits[-1].feature_end = hit_end
            hit_start_found = False
            hit_end_found = False

        offset += len(query)

    return hits
