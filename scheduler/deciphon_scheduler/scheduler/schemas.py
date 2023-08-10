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
    HMMName,
    SnapName,
    Gencode,
)
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
    id: int

    state: Optional[JobState] = None
    progress: Optional[int] = None
    error: Optional[str] = None
    exec_started: Optional[datetime] = None
    exec_ended: Optional[datetime] = None


class HMMFile(HMMName):
    @property
    def db_name(self):
        return DBName(name=self.name[:-4] + ".dcp")

    @property
    def path_type(self):
        return fastapi.Path(
            title="HMM file",
            pattern=HMM_NAME_PATTERN,
            max_length=NAME_MAX_LENGTH,
        )


class DBFile(DBName):
    gencode: Gencode
    epsilon: float

    @property
    def hmm_name(self):
        return HMMName(name=self.name[:-4] + ".hmm")

    @property
    def path_type(self):
        return fastapi.Path(
            title="DB file",
            pattern=DB_NAME_PATTERN,
            max_length=NAME_MAX_LENGTH,
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
    hmm: HMMFile
    db: DBFile

    @classmethod
    def create(cls, hmm: HMMFile, gencode: Gencode, epsilon: float):
        db = DBFile(name=hmm.db_name.name, gencode=gencode, epsilon=epsilon)
        return cls(hmm=hmm, db=db)


class HMMRead(BaseModel):
    id: int
    job: JobRead
    file: HMMFile


class DBCreate(BaseModel):
    file: DBFile


class DBRead(BaseModel):
    id: int
    hmm: HMMRead
    file: DBFile


class SeqCreate(BaseModel):
    name: str
    data: str


class SeqRead(BaseModel):
    id: int
    name: str
    data: str


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
