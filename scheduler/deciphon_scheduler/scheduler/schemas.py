from datetime import datetime
from enum import Enum
from pathlib import Path
from typing import Optional

from pydantic import BaseModel, Field

from deciphon_scheduler.scheduler.validation import (
    FILE_NAME_MAX_LENGTH,
    DB_FILE_NAME_PATTERN,
    HMM_FILE_NAME_PATTERN,
    SNAP_FILE_NAME_PATTERN,
)


class JobType(Enum):
    hmm = "hmm"
    scan = "scan"


class JobState(Enum):
    pend = "pend"
    run = "run"
    done = "done"
    fail = "fail"


def _file_name_pattern(ext: str):
    return r"^[0-9a-zA-Z_\-.][0-9a-zA-Z_\-. ]+\." + ext + "$"


_FILE_NAME_MAX = 128


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


class HMMFileName(BaseModel):
    name: str = Field(pattern=HMM_FILE_NAME_PATTERN, max_length=FILE_NAME_MAX_LENGTH)

    @property
    def db_file_name(self):
        return DBFileName(name=str(Path(self.name).with_suffix(".dcp")))


class DBFileName(BaseModel):
    name: str = Field(pattern=DB_FILE_NAME_PATTERN, max_length=FILE_NAME_MAX_LENGTH)

    @property
    def hmm_file_name(self):
        return HMMFileName(name=str(Path(self.name).with_suffix(".hmm")))


class SnapFileName(BaseModel):
    name: str = Field(pattern=SNAP_FILE_NAME_PATTERN, max_length=FILE_NAME_MAX_LENGTH)


class HMMRead(BaseModel):
    id: int
    job_id: int
    file: HMMFileName


class DBCreate(BaseModel):
    file: DBFileName


class DBRead(BaseModel):
    id: int
    hmm_id: int
    file: DBFileName


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
    db_id: int
    multi_hits: bool
    hmmer3_compat: bool
    seqs: list[SeqRead]
