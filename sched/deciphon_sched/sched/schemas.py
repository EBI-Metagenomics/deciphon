from __future__ import annotations

import fastapi
from deciphon_schema import (
    DB_NAME_PATTERN,
    HMM_NAME_PATTERN,
    NAME_MAX_LENGTH,
    DBName,
    JobState,
)
from pydantic import BaseModel


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


class DBCreate(BaseModel):
    file: DBName


class SeqCreate(BaseModel):
    name: str
    data: str


class ScanCreate(BaseModel):
    db_id: int
    multi_hits: bool
    hmmer3_compat: bool
    seqs: list[SeqCreate]
