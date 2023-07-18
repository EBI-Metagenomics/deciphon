from __future__ import annotations

from typing import List, Optional

from sqlmodel import Field, Relationship

from deciphon_api.models.db import DBBase
from deciphon_api.models.hmm import HMMBase
from deciphon_api.models.job import JobBase
from deciphon_api.models.prod import ProdBase
from deciphon_api.models.scan import ScanBase
from deciphon_api.models.seq import SeqBase
from deciphon_api.models.snap import SnapBase

NOLIST = {"sa_relationship_kwargs": {"uselist": False}}
CASCADE = {"sa_relationship_kwargs": {"cascade": "all,delete,delete-orphan"}}
NOLIST_CASCADE = {"sa_relationship_kwargs": {"uselist": False, "cascade": "all,delete"}}


class Prod(ProdBase, table=True):
    id: Optional[int] = Field(default=None, primary_key=True)
    snap_id: Optional[int] = Field(default=None, foreign_key="snap.id")

    snap: Snap = Relationship(back_populates="prods", **NOLIST)
    seq: Seq = Relationship(back_populates="prod", **NOLIST)


class Snap(SnapBase, table=True):
    id: Optional[int] = Field(default=None, primary_key=True)

    scan: Scan = Relationship(back_populates="snap", **NOLIST)
    prods: List[Prod] = Relationship(back_populates="snap", **CASCADE)


class Seq(SeqBase, table=True):
    id: Optional[int] = Field(default=None, primary_key=True)
    scan_id: Optional[int] = Field(default=None, foreign_key="scan.id")

    scan: Scan = Relationship(back_populates="seqs", **NOLIST)
    prod: Optional[Prod] = Relationship(back_populates="seq", **NOLIST)


class Scan(ScanBase, table=True):
    id: Optional[int] = Field(default=None, primary_key=True)
    job_id: Optional[int] = Field(default=None, foreign_key="job.id")

    db: DB = Relationship(back_populates="scans", **NOLIST)
    job: Job = Relationship(back_populates="scan", **NOLIST_CASCADE)
    seqs: List[Seq] = Relationship(back_populates="scan", **CASCADE)
    snap: Snap = Relationship(back_populates="scan", **NOLIST_CASCADE)


class DB(DBBase, table=True):
    id: Optional[int] = Field(default=None, primary_key=True)
    hmm_id: Optional[int] = Field(default=None, foreign_key="hmm.id")

    hmm: HMM = Relationship(back_populates="db", **NOLIST)
    scans: List[Scan] = Relationship(back_populates="db")


class HMM(HMMBase, table=True):
    id: Optional[int] = Field(default=None, primary_key=True)
    job_id: Optional[int] = Field(default=None, foreign_key="job.id")

    job: Job = Relationship(back_populates="hmm", **NOLIST_CASCADE)
    db: Optional[DB] = Relationship(back_populates="hmm", **NOLIST_CASCADE)


class Job(JobBase, table=True):
    id: Optional[int] = Field(default=None, primary_key=True)

    hmm: Optional[HMM] = Relationship(back_populates="job", **NOLIST_CASCADE)
    scan: Optional[Scan] = Relationship(back_populates="job", **NOLIST_CASCADE)
