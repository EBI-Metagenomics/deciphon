from __future__ import annotations

from datetime import datetime
from enum import Enum
from typing import Optional

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

    @classmethod
    def create(cls, type: JobType, state=JobState.pend):
        return cls(
            type=type,
            state=state,
            progress=0,
            error="",
            submission=datetime.utcnow(),
        )


class HMM(Base):
    __tablename__ = "hmm"

    id: Mapped[int] = mapped_column(primary_key=True)
    job_id: Mapped[Optional[int]] = mapped_column(ForeignKey("job.id"))

    job: Mapped[Job] = relationship(back_populates="hmm")
    db: Mapped[Optional[DB]] = relationship(back_populates="hmm")

    @classmethod
    def create(cls):
        return cls(job=Job.create(type=JobType.hmm))


class DB(Base):
    __tablename__ = "db"

    id: Mapped[int] = mapped_column(primary_key=True)
    hmm_id: Mapped[int] = mapped_column(ForeignKey("hmm.id"))

    hmm: Mapped[HMM] = relationship(back_populates="db")
    scans: Mapped[list[Scan]] = relationship(back_populates="db")

    @classmethod
    def create(cls, hmm: HMM):
        return cls(hmm=HMM.create())


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
