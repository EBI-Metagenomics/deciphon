from functools import partial
from itertools import accumulate, chain
from typing import Optional

import tabulate
from hmmer_tables.query import DomAnnot, read_query
from pydantic import BaseModel

from deciphon_snap.match import Match, MatchList
from deciphon_snap.prod import H3Result, Prod
from deciphon_snap.snap_file import SnapFile

__all__ = ["view_alignments"]


class DeciStep(BaseModel):
    match: Match


class HMMERStep(BaseModel):
    hmm_pos: int
    hmm_cs: str
    amino_pos: int
    amino_cs: str
    amino: str
    match: str
    score: str


class AssocStep(BaseModel):
    deci: Optional[DeciStep]
    hmmer: Optional[HMMERStep]


def make_deciphon_steps(match_list: MatchList):
    return list(enumerate([DeciStep(match=x) for x in match_list if len(x.amino) > 0]))


def make_hmmer_annot(h3result: H3Result):
    hmmer_query = read_query(stream=h3result.domains.splitlines())
    assert len(hmmer_query.domains) == 1
    return hmmer_query.domains[0]


def make_hmmer_steps(annot: DomAnnot):
    hmmer: list[tuple[int, HMMERStep]] = []
    for x in annot.aligns:
        a = x.align
        core_positions = list(range(a.core_interval.start, a.core_interval.stop + 1))
        start = a.query_interval.to_pyinterval().start
        stop = a.query_interval.to_pyinterval().stop
        offset = start - 1
        amino_pos = [offset + i for i in accumulate([int(i != "-") for i in a.query])]
        assert amino_pos[0] == start
        assert amino_pos[-1] + 1 == stop
        for i in range(len(core_positions)):
            step = HMMERStep(
                hmm_pos=core_positions[i],
                hmm_cs=a.hmm_cs[i],
                amino_pos=amino_pos[i],
                amino_cs=a.query_cs[i],
                amino=a.query[i],
                match=a.match[i],
                score=a.score[i],
            )
            hmmer.append((amino_pos[i], step))
    return hmmer


def assoc_steps(deci: list[tuple[int, DeciStep]], hmmer: list[tuple[int, HMMERStep]]):
    steps: list[AssocStep] = []
    di = 0
    hi = 0
    while di < len(deci) and hi < len(hmmer):
        deci_pos = deci[di][0]
        hmmer_pos = hmmer[hi][0]

        if deci_pos < hmmer_pos:
            steps.append(AssocStep(deci=deci[di][1], hmmer=None))
            di += 1

        if hmmer_pos < deci_pos:
            steps.append(AssocStep(deci=None, hmmer=hmmer[hi][1]))
            hi += 1

        if deci_pos == hmmer_pos:
            steps.append(AssocStep(deci=deci[di][1], hmmer=hmmer[hi][1]))
            di += 1
            hi += 1

    while di < len(deci):
        steps.append(AssocStep(deci=deci[di][1], hmmer=None))
        di += 1

    while hi < len(hmmer):
        steps.append(AssocStep(deci=None, hmmer=hmmer[hi][1]))
        hi += 1

    return steps


def flat(x):
    return list(chain.from_iterable(x))


def view_alignment(prod: Prod):
    assert prod.h3result is not None
    annot = make_hmmer_annot(prod.h3result)
    txt = "Alignments for each domain:\n"
    if len(annot.aligns) == 0:
        txt += annot.head.strip() + "\n"
        return txt

    deci = make_deciphon_steps(prod.matches)
    hmmer = make_hmmer_steps(annot)

    align_head = annot.aligns[0].head
    txt += align_head.strip() + "\n"

    profile = annot.aligns[0].align.profile
    query_name = annot.aligns[0].align.query_name

    steps: list[AssocStep] = assoc_steps(deci, hmmer)

    def grab_amino(x):
        if x.hmmer is not None and x.deci is not None:
            return x.deci.match.amino
        if x.hmmer is not None and x.deci is None:
            return "-"
        return ""

    def grab_query(x, i):
        if x.hmmer is not None and x.deci is not None:
            q = x.deci.match.query
            return q[i] if len(q) > i else "."
        if x.hmmer is not None and x.deci is None:
            return "-"
        return ""

    query_pos = []
    curr_pos = 0
    for x in steps:
        if x.hmmer is not None and x.deci is not None:
            curr_pos = x.deci.match.position
            query_pos.append(curr_pos)
            curr_pos += len(x.deci.match.query)
        if x.hmmer is not None and x.deci is None:
            query_pos.append(curr_pos)

    hmm_cs = flat(("" if x.hmmer is None else x.hmmer.hmm_cs) for x in steps)
    query = flat(("" if x.hmmer is None else x.hmmer.amino) for x in steps)
    match = flat(("" if x.hmmer is None else x.hmmer.match) for x in steps)

    amino = flat(grab_amino(x) for x in steps)
    q0 = flat(partial(grab_query, i=0)(x) for x in steps)
    q1 = flat(partial(grab_query, i=1)(x) for x in steps)
    q2 = flat(partial(grab_query, i=2)(x) for x in steps)
    q3 = flat(partial(grab_query, i=3)(x) for x in steps)
    q4 = flat(partial(grab_query, i=4)(x) for x in steps)
    score = flat(("" if x.hmmer is None else x.hmmer.score) for x in steps)

    hmm_pos = []
    for x in steps:
        if x.hmmer is not None:
            hmm_pos.append(x.hmmer.hmm_pos)

    amino_pos = []
    for x in steps:
        if x.hmmer is not None:
            amino_pos.append(x.hmmer.amino_pos)

    table = []
    n = len(hmm_cs)
    COLS = 96
    width = min(COLS, n)
    for i in range(0, n, COLS):
        sl = slice(i, min(i + COLS, n))
        pad = "&" * (width - (sl.stop - sl.start))
        hmm_pos_left = hmm_pos[sl][0]
        hmm_pos_right = hmm_pos[sl][-1]
        amino_pos_left = amino_pos[sl][0] + 1
        amino_pos_right = amino_pos[sl][-1] + 1
        query_pos_left = query_pos[sl][0] + 1
        query_pos_right = query_pos[sl][-1] + 1
        row = [
            [None, None, "".join(hmm_cs[sl]) + pad, "CS"],
            [profile, hmm_pos_left, "".join(query[sl]) + pad, hmm_pos_right],
            [None, None, "".join(match[sl]) + pad, None],
            [None, amino_pos_left, "".join(amino[sl]) + pad, amino_pos_right],
            [query_name, query_pos_left, "".join(q0[sl]) + pad, query_pos_right],
            [None, None, "".join(q1[sl]) + pad, None],
            [None, None, "".join(q2[sl]) + pad, None],
            [None, None, "".join(q3[sl]) + pad, None],
            [None, None, "".join(q4[sl]) + pad, None],
            [None, None, "".join(score[sl]) + pad, "PP"],
        ]
        table += row + [[None, None, None, None]]
    tablefmt = tabulate.simple_separated_format(" ")
    txt += str(
        tabulate.tabulate(
            table, tablefmt=tablefmt, colalign=("right", "right", "left", "left")
        )
    )
    txt = txt.replace("&", "") + "\n"
    return txt


def view_alignments(snap: SnapFile):
    txt = []
    for prod in snap.products:
        txt.append(view_alignment(prod))
    return "\n".join(txt)
