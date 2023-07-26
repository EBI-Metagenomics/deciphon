from __future__ import annotations

from datetime import datetime
from enum import Enum
from typing import Optional

from pydantic import BaseModel, Field
from sqlalchemy import ForeignKey
from sqlalchemy.orm import DeclarativeBase, Mapped, mapped_column, relationship


class Base(DeclarativeBase):
    def model_dump(self):
        return {field.name: getattr(self, field.name) for field in self.__table__.c}

    def __repr__(self):
        params = ", ".join(f"{k}={v}" for k, v in self.model_dump().items())
        return f"{self.__class__.__name__}({params})"

    def __str__(self):
        return repr(self)


class JobType(Enum):
    hmm = "hmm"
    scan = "scan"


class JobState(Enum):
    pend = "pend"
    run = "run"
    done = "done"
    fail = "fail"


class Job(Base):
    __tablename__ = "job"

    id: Mapped[int] = mapped_column(primary_key=True)

    hmm: Mapped[Optional[HMM]] = relationship(back_populates="job")
    scan: Mapped[Optional[Scan]] = relationship(back_populates="job")

    type: Mapped[JobType]
    state: Mapped[JobState]
    progress: Mapped[int]
    error: Mapped[str]
    submission: Mapped[datetime]
    exec_started: Mapped[Optional[datetime]]
    exec_ended: Mapped[Optional[datetime]]

    @classmethod
    def create(cls, type: JobType, state=JobState.pend):
        return cls(
            type=type,
            state=state,
            progress=0,
            error="",
            submission=datetime.utcnow(),
        )


def _file_name_pattern(ext: str):
    return r"^[0-9a-zA-Z_\-.][0-9a-zA-Z_\-. ]+\." + ext + "$"


_SHA256_PATTERN = r"^[0123456789abcdef]{64}$"


class HMMFile(BaseModel):
    name: str = Field(pattern=_file_name_pattern("hmm"))
    sha256: str = Field(pattern=_SHA256_PATTERN)


class DBFile(BaseModel):
    name: str = Field(pattern=_file_name_pattern("dcp"))
    sha256: str = Field(pattern=_SHA256_PATTERN)


class HMM(Base):
    __tablename__ = "hmm"

    id: Mapped[int] = mapped_column(primary_key=True)
    job_id: Mapped[Optional[int]] = mapped_column(ForeignKey("job.id"))

    job: Mapped[Job] = relationship(back_populates="hmm")
    db: Mapped[Optional[DB]] = relationship(back_populates="hmm")

    file_name: Mapped[str]
    file_sha256: Mapped[str]

    @classmethod
    def create(cls, file: HMMFile):
        job = Job.create(type=JobType.hmm)
        return cls(
            job=job,
            file_name=file.name,
            file_sha256=file.sha256,
        )


class DB(Base):
    __tablename__ = "db"

    id: Mapped[int] = mapped_column(primary_key=True)
    hmm_id: Mapped[int] = mapped_column(ForeignKey("hmm.id"))

    hmm: Mapped[HMM] = relationship(back_populates="db")
    scans: Mapped[list[Scan]] = relationship(back_populates="db")

    file_name: Mapped[str]
    file_sha256: Mapped[str]

    @classmethod
    def create(cls, hmm: HMM, file: DBFile):
        return cls(hmm=hmm, file_name=file.name, file_sha256=file.sha256)


class Scan(Base):
    __tablename__ = "scan"

    id: Mapped[int] = mapped_column(primary_key=True)
    job_id: Mapped[int] = mapped_column(ForeignKey("job.id"))
    db_id: Mapped[int] = mapped_column(ForeignKey("db.id"))

    job: Mapped[Job] = relationship(back_populates="scan")
    db: Mapped[DB] = relationship(back_populates="scans")

    @classmethod
    def create(cls, db: DB):
        return cls(job=Job.create(type=JobType.scan), db=db)
