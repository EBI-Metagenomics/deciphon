from __future__ import annotations

from datetime import datetime
from enum import Enum
from pathlib import Path
from typing import Optional

from deciphon_core.schema import DBName, Gencode, HMMName
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


class HMM(BaseModel):
    id: int
    job: JobRead
    file: HMMFileName


class HMMFileName(BaseModel):
    name: str

    @property
    def db_file_name(self):
        return DBFileName(name=str(Path(self.name).with_suffix(".dcp")))


class DBFileName(BaseModel):
    name: str

    @property
    def hmm_file_name(self):
        return HMMFileName(name=str(Path(self.name).with_suffix(".hmm")))


class HMMFile(HMMName):
    ...


class DBFile(DBName):
    gencode: Gencode
    epsilon: float


class PressRequest(BaseModel):
    job_id: int
    hmm: HMMFile
    db: DBFile


class Task(Enum):
    press = "press"
    scan = "scan"


class JobUpdate(BaseModel):
    id: int
    state: JobState
    progress: int
    error: str


class Seq(BaseModel):
    name: str
    data: str


class Scan(BaseModel):
    db_id: int
    multi_hits: bool
    hmmer3_compat: bool
    seqs: list[Seq]
