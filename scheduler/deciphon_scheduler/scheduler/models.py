from __future__ import annotations

from sqlalchemy import select
from collections.abc import Iterable
from datetime import datetime
from typing import Optional

from sqlalchemy import ForeignKey
from sqlalchemy.orm import Mapped, Session, mapped_column, relationship

from ..models import BaseModel
from .schemas import DBFile, HMMFile, HMMRead, JobState, JobType, DBRead, JobRead


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

    def set_done(self):
        self.state = JobState.done
        self.progress = 100
        now = datetime.now()
        if not self.exec_started:
            self.exec_started = now
        self.exec_ended = now

    def read_model(self):
        return JobRead(
            id=self.id,
            type=self.type,
            state=self.state,
            progress=self.progress,
            error=self.error,
            submission=self.submission,
            exec_started=self.exec_started,
            exec_ended=self.exec_ended,
        )

    @staticmethod
    def get_by_id(session: Session, id: int):
        x = session.execute(select(Job).where(Job.id == id)).one_or_none()
        return x if x is None else x._tuple()[0]

    @staticmethod
    def get_all(session: Session):
        return [x._tuple()[0] for x in session.execute(select(Job)).all()]


class HMM(BaseModel):
    __tablename__ = "hmm"

    id: Mapped[int] = mapped_column(primary_key=True)
    job_id: Mapped[Optional[int]] = mapped_column(ForeignKey("job.id"))

    job: Mapped[Job] = relationship(back_populates="hmm")
    db: Mapped[Optional[DB]] = relationship(back_populates="hmm")

    file_name: Mapped[str] = mapped_column(unique=True)
    file_sha256: Mapped[str]

    @classmethod
    def create(cls, file: HMMFile):
        job = Job.create(type=JobType.hmm)
        return cls(
            job=job,
            file_name=file.name,
            file_sha256=file.sha256,
        )

    def read_model(self):
        file = HMMFile(name=self.file_name, sha256=self.file_sha256)
        return HMMRead(id=self.id, job_id=self.job_id, file=file)

    @staticmethod
    def get_by_id(session: Session, id: int):
        x = session.execute(select(HMM).where(HMM.id == id)).one_or_none()
        return x if x is None else x._tuple()[0]

    @staticmethod
    def get_by_file_name(session: Session, file_name: str):
        x = session.execute(select(HMM).where(HMM.file_name == file_name)).one_or_none()
        return x if x is None else x._tuple()[0]

    @staticmethod
    def get_all(session: Session):
        return [x._tuple()[0] for x in session.execute(select(HMM)).all()]


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

    def read_model(self):
        file = DBFile(name=self.file_name, sha256=self.file_sha256)
        return DBRead(id=self.id, hmm_id=self.hmm_id, file=file)

    @staticmethod
    def get_by_id(session: Session, id: int):
        x = session.execute(select(DB).where(DB.id == id)).one_or_none()
        return x if x is None else x._tuple()[0]

    @staticmethod
    def get_by_file_name(session: Session, file_name: str):
        x = session.execute(select(DB).where(DB.file_name == file_name)).one_or_none()
        return x if x is None else x._tuple()[0]

    @staticmethod
    def get_all(session: Session):
        return [x._tuple()[0] for x in session.execute(select(DB)).all()]


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
