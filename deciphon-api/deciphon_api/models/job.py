from __future__ import annotations

from datetime import datetime
from enum import Enum
from typing import Optional

from sqlmodel import Field, SQLModel


class JobType(Enum):
    hmm = "hmm"
    scan = "scan"


class JobState(Enum):
    pend = "pend"
    run = "run"
    done = "done"
    fail = "fail"


class JobBase(SQLModel):
    type: JobType = Field(nullable=False)
    state: JobState = Field(default=JobState.pend, nullable=False)
    progress: int = Field(default=0, ge=0, le=100, nullable=False)
    error: str = Field(default="", nullable=False)
    submission: datetime = Field(default_factory=datetime.now)
    exec_started: Optional[datetime] = Field(default=None, nullable=True)
    exec_ended: Optional[datetime] = Field(default=None, nullable=True)

    def set_done(self):
        self.state = JobState.done
        self.progress = 100
        now = datetime.now()
        if not self.exec_started:
            self.exec_started = now
        self.exec_ended = now


class JobCreate(JobBase):
    ...


class JobRead(JobBase):
    id: int


class JobUpdate(SQLModel):
    state: Optional[JobState] = None
    progress: Optional[int] = None
    error: Optional[str] = None
    exec_started: Optional[datetime] = None
    exec_ended: Optional[datetime] = None
