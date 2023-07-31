from datetime import datetime
from enum import Enum
from pathlib import Path
from typing import Optional

from pydantic import BaseModel, Field


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
    name: str = Field(pattern=_file_name_pattern("hmm"), max_length=_FILE_NAME_MAX)

    @property
    def db_file_name(self):
        return DBFileName(name=str(Path(self.name).with_suffix(".dcp")))


class DBFileName(BaseModel):
    name: str = Field(pattern=_file_name_pattern("dcp"), max_length=_FILE_NAME_MAX)

    @property
    def hmm_file_name(self):
        return HMMFileName(name=str(Path(self.name).with_suffix(".hmm")))


class SnapFileName(BaseModel):
    name: str = Field(pattern=_file_name_pattern("dcs"), max_length=_FILE_NAME_MAX)


class HMMCreate(BaseModel):
    file: HMMFileName


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
