from __future__ import annotations

from collections.abc import Iterable
from datetime import datetime
from typing import Optional

from sqlalchemy import ForeignKey
from sqlalchemy.orm import Mapped, mapped_column, relationship

from ..models import BaseModel
from .schemas import DBFile, HMMFile, JobState, JobType


class Job(BaseModel):
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


class HMM(BaseModel):
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


class DB(BaseModel):
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


class Seq(BaseModel):
    __tablename__ = "seq"

    id: Mapped[int] = mapped_column(primary_key=True)
    scan_id: Mapped[int] = mapped_column(ForeignKey("scan.id"))

    scan: Mapped[Scan] = relationship(back_populates="seqs")

    name: Mapped[str]
    data: Mapped[str]


class Snap(BaseModel):
    __tablename__ = "snap"

    id: Mapped[int] = mapped_column(primary_key=True)
    scan_id: Mapped[int] = mapped_column(ForeignKey("scan.id"))

    scan: Mapped[Scan] = relationship(back_populates="snap")

    file_name: Mapped[str]
    file_sha256: Mapped[str]


class Scan(BaseModel):
    __tablename__ = "scan"

    id: Mapped[int] = mapped_column(primary_key=True)
    job_id: Mapped[int] = mapped_column(ForeignKey("job.id"))
    db_id: Mapped[int] = mapped_column(ForeignKey("db.id"))

    job: Mapped[Job] = relationship(back_populates="scan")
    seqs: Mapped[list[Seq]] = relationship(back_populates="scan")
    snap: Mapped[Optional[Snap]] = relationship(back_populates="scan")
    db: Mapped[DB] = relationship(back_populates="scans")

    @classmethod
    def create(cls, db: DB, seqs: Iterable[Seq]):
        return cls(job=Job.create(type=JobType.scan), db=db, seqs=list(seqs))
