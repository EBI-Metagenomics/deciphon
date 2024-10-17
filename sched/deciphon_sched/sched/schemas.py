from __future__ import annotations

from datetime import datetime
from enum import Enum
from typing import Optional

import fastapi
from deciphon_core.schema import (
    DB_NAME_PATTERN,
    HMM_NAME_PATTERN,
    NAME_MAX_LENGTH,
    SNAP_NAME_PATTERN,
    DBName,
    Gencode,
    HMMName,
    SnapName,
)
from deciphon_snap.prod import Prod
from pydantic import BaseModel


class JobType(Enum):
    hmm = "hmm"
    scan = "scan"


class JobState(Enum):
    pend = "pend"
    run = "run"
    done = "done"
    fail = "fail"


class JobRead(BaseModel):
    id: int
    type: JobType
    state: JobState
    progress: int
    error: str
    submission: datetime
    exec_started: Optional[datetime]
    exec_ended: Optional[datetime]


class JobUpdate(BaseModel):
    state: JobState
    progress: int
    error: str


HMMFilePathType = fastapi.Path(
    title="HMM file", pattern=HMM_NAME_PATTERN, max_length=NAME_MAX_LENGTH
)


DBFilePathType = fastapi.Path(
    title="DB file", pattern=DB_NAME_PATTERN, max_length=NAME_MAX_LENGTH
)


class SnapFile(SnapName):
    @property
    def path_type(self):
        return fastapi.Path(
            title="Snap file",
            pattern=SNAP_NAME_PATTERN,
            max_length=NAME_MAX_LENGTH,
        )


class PressRequest(BaseModel):
    job_id: int
    hmm: HMMName
    db: DBName
    gencode: Gencode
    epsilon: float

    @classmethod
    def create(cls, job_id: int, hmm: HMMName, gencode: Gencode, epsilon: float):
        return cls(
            job_id=job_id, hmm=hmm, db=hmm.db_name, gencode=gencode, epsilon=epsilon
        )


class ScanRequest(BaseModel):
    id: int
    job_id: int
    hmm: HMMName
    db: DBName
    multi_hits: bool
    hmmer3_compat: bool
    seqs: list[SeqRead]

    @classmethod
    def create(cls, scan: ScanRead):
        return cls(
            id=scan.id,
            job_id=scan.job.id,
            hmm=HMMName(name=scan.db.hmm.file.name),
            db=scan.db.file,
            multi_hits=scan.multi_hits,
            hmmer3_compat=scan.hmmer3_compat,
            seqs=scan.seqs,
        )


class HMMRead(BaseModel):
    id: int
    job: JobRead
    file: HMMName


class DBCreate(BaseModel):
    file: DBName


class DBRead(BaseModel):
    id: int
    hmm: HMMRead
    file: DBName


class SeqCreate(BaseModel):
    name: str
    data: str


class SeqRead(BaseModel):
    id: int
    name: str
    data: str


class SnapRead(BaseModel):
    id: int
    size: int


class ScanCreate(BaseModel):
    db_id: int
    multi_hits: bool
    hmmer3_compat: bool
    seqs: list[SeqCreate]


class ScanRead(BaseModel):
    id: int
    job: JobRead
    db: DBRead
    multi_hits: bool
    hmmer3_compat: bool
    seqs: list[SeqRead]


class ProdRead(BaseModel):
    seq_id: int
    profile: str
    abc: str
    lrt: float
    evalue: float

    @classmethod
    def make(cls, x: Prod):
        return cls(
            seq_id=x.seq_id,
            profile=x.profile,
            abc=x.abc,
            lrt=x.lrt,
            evalue=x.evalue,
        )
