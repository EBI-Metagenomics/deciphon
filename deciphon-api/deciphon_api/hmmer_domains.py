from __future__ import annotations

from deciphon_api.hmmer_path import HMMERPath, HMMERStep
from deciphon_api.liner import Liner

__all__ = ["read_hmmer_paths", "read_hmmer_headers"]


def is_header(row: str):
    return row.replace(" ", "").startswith("==")


def reach_header_line(data: Liner):
    for row in data:
        row = row.strip()
        if len(row) == 0:
            continue
        if is_header(row):
            return


def rowit(data: Liner):
    for row in data:
        row = row.strip()
        if len(row) == 0:
            continue
        yield row


def stepit(data: Liner):
    i = rowit(data)
    while True:
        try:
            row = next(i)
        except StopIteration:
            break

        if is_header(row):
            break

        if row[row.rfind(" ") :].strip() == "CS":
            last = row.rfind(" ")
            hmm_cs = row[:last].strip()
            row = next(i)
        else:
            hmm_cs = ""

        acc, start = row.split(maxsplit=2)[:2]
        offset = row.find(acc) + len(acc)
        offset = row.find(start, offset) + len(start)
        last = row.rfind(" ")
        tgt_cs = row[offset:last].strip()

        row = next(i)
        match = row.strip()

        row = next(i)
        start = row.split()[0]
        offset = row.find(start) + len(start)
        last = row.rfind(" ")
        tgt = row[offset:last].strip()

        row = next(i)
        last = row.rfind(" ")
        assert row[last:].strip() == "PP"
        score = row[:last].strip()

        if len(hmm_cs) == 0:
            hmm_cs = "?" * len(tgt_cs)
        assert len(hmm_cs) == len(tgt_cs) == len(match) == len(tgt) == len(score)
        for x in zip(hmm_cs, tgt_cs, match, tgt, score):
            yield HMMERStep(*x)


def pathit(data: Liner):
    reach_header_line(data)

    while True:
        y = [x for x in stepit(data)]
        if len(y) == 0:
            break
        yield HMMERPath(y)


def read_hmmer_paths(data: Liner):
    return [x for x in pathit(data)]


def read_hmmer_headers(data: Liner):
    for row in data:
        row = row.strip()
        if len(row) == 0:
            continue
        if is_header(row):
            yield row.strip()
