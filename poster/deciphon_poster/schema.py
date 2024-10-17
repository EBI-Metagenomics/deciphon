from __future__ import annotations

from enum import Enum
from typing import Any

from pydantic import BaseModel, HttpUrl


class JobType(Enum):
    hmm = "hmm"
    scan = "scan"


class JobState(Enum):
    pend = "pend"
    run = "run"
    done = "done"
    fail = "fail"


class JobUpdate(BaseModel):
    id: int
    state: JobState
    progress: int
    error: str

    @classmethod
    def run(cls, job_id: int, progress: int):
        return cls(
            id=job_id,
            state=JobState.run,
            progress=progress,
            error="",
        )

    @classmethod
    def fail(cls, job_id: int, error: str):
        return cls(
            id=job_id,
            state=JobState.fail,
            progress=0,
            error=error,
        )


class Seq(BaseModel):
    name: str
    data: str


class SeqRequest(Seq):
    id: int


class Scan(BaseModel):
    db_id: int
    multi_hits: bool
    hmmer3_compat: bool
    seqs: list[Seq]


class UploadPost(BaseModel):
    url: HttpUrl
    fields: dict[str, Any]

    @property
    def url_string(self):
        return self.url.unicode_string()
