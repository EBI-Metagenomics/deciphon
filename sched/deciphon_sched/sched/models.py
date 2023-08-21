from __future__ import annotations

from collections.abc import Iterable
from datetime import datetime
from typing import Optional

from deciphon_core.schema import Gencode
from sqlalchemy import ForeignKey, select
from sqlalchemy.orm import DeclarativeBase, Mapped, Session, mapped_column, relationship
from deciphon_sched.errors import JobStateTransitionError

from deciphon_sched.sched.schemas import (
    DBFile,
    DBRead,
    HMMFile,
    HMMRead,
    JobRead,
    JobState,
    JobType,
    ScanRead,
    SeqRead,
    SnapRead,
)


def metadata():
    return BaseModel.metadata


class BaseModel(DeclarativeBase):
    def model_dump(self):
        return {field.name: getattr(self, field.name) for field in self.__table__.c}

    def __repr__(self):
        params = ", ".join(f"{k}={v}" for k, v in self.model_dump().items())
        return f"{self.__class__.__name__}({params})"

    def __str__(self):
        return repr(self)


DELETE = "save-update, merge, delete"


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
    def create(cls, type: JobType):
        return cls(
            type=type,
            state=JobState.pend,
            progress=0,
            error="",
            submission=datetime.utcnow(),
        )

    def set_pend(self):
        raise JobStateTransitionError(str(self.state), str(JobState.pend))

    def set_run(self, progress: int):
        if self.state in [JobState.done, JobState.fail]:
            raise JobStateTransitionError(str(self.state), str(JobState.run))
        self.state = JobState.run
        self.progress = progress
        if self.exec_started is None:
            self.exec_started = datetime.now()

    def set_done(self):
        if self.state in [JobState.done, JobState.fail]:
            raise JobStateTransitionError(str(self.state), str(JobState.done))
        self.state = JobState.done
        self.progress = 100
        if self.exec_started is None:
            self.exec_started = datetime.now()
        self.exec_ended = datetime.now()

    def set_fail(self, error: str):
        if self.state in [JobState.done, JobState.fail]:
            raise JobStateTransitionError(str(self.state), str(JobState.fail))
        self.state = JobState.fail
        self.error = error
        if self.exec_started is None:
            self.exec_started = datetime.now()
        self.exec_ended = datetime.now()

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
    job_id: Mapped[int] = mapped_column(ForeignKey("job.id"))

    job: Mapped[Job] = relationship(back_populates="hmm", cascade=DELETE)
    db: Mapped[Optional[DB]] = relationship(back_populates="hmm")

    name: Mapped[str] = mapped_column(unique=True)

    @classmethod
    def create(cls, file: HMMFile):
        return cls(job=Job.create(type=JobType.hmm), name=file.name)

    def read_model(self):
        file = HMMFile(name=self.name)
        return HMMRead(id=self.id, job=self.job.read_model(), file=file)

    @staticmethod
    def get_by_id(session: Session, id: int):
        x = session.execute(select(HMM).where(HMM.id == id)).one_or_none()
        return x if x is None else x._tuple()[0]

    @staticmethod
    def get_by_name(session: Session, name: str):
        x = session.execute(select(HMM).where(HMM.name == name)).one_or_none()
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

    name: Mapped[str]
    gencode: Mapped[int]
    epsilon: Mapped[float]

    @classmethod
    def create(cls, hmm: HMM, file: DBFile):
        return cls(
            hmm=hmm,
            name=file.name,
            gencode=int(file.gencode),
            epsilon=file.epsilon,
        )

    def read_model(self):
        file = DBFile(
            name=self.name, gencode=Gencode(self.gencode), epsilon=self.epsilon
        )
        return DBRead(id=self.id, hmm=self.hmm.read_model(), file=file)

    @staticmethod
    def get_by_id(session: Session, id: int):
        x = session.execute(select(DB).where(DB.id == id)).one_or_none()
        return x if x is None else x._tuple()[0]

    @staticmethod
    def get_by_name(session: Session, name: str):
        x = session.execute(select(DB).where(DB.name == name)).one_or_none()
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

    @classmethod
    def create(cls, name: str, data: str):
        return cls(name=name, data=data)

    def read_model(self):
        return SeqRead(id=self.id, name=self.name, data=self.data)

    @staticmethod
    def get_by_id(session: Session, id: int):
        x = session.execute(select(Seq).where(Seq.id == id)).one_or_none()
        return x if x is None else x._tuple()[0]

    @staticmethod
    def get_all(session: Session):
        return [x._tuple()[0] for x in session.execute(select(Seq)).all()]


class Snap(BaseModel):
    __tablename__ = "snap"

    id: Mapped[int] = mapped_column(primary_key=True)
    scan_id: Mapped[int] = mapped_column(ForeignKey("scan.id"))

    scan: Mapped[Scan] = relationship(back_populates="snap")

    data: Mapped[bytes]

    @classmethod
    def create(cls, scan_id: int, data: bytes):
        return cls(scan_id=scan_id, data=data)

    def read_model(self):
        return SnapRead(id=self.id, size=len(self.data))

    @staticmethod
    def get_by_scan_id(session: Session, id: int):
        x = session.execute(select(Snap).where(Snap.scan_id == id)).one_or_none()
        return x if x is None else x._tuple()[0]

    @staticmethod
    def get_all(session: Session):
        return [x._tuple()[0] for x in session.execute(select(Snap)).all()]


class Scan(BaseModel):
    __tablename__ = "scan"

    id: Mapped[int] = mapped_column(primary_key=True)
    job_id: Mapped[int] = mapped_column(ForeignKey("job.id"))
    db_id: Mapped[int] = mapped_column(ForeignKey("db.id"))

    multi_hits: Mapped[bool]
    hmmer3_compat: Mapped[bool]

    job: Mapped[Job] = relationship(back_populates="scan", cascade=DELETE)
    seqs: Mapped[list[Seq]] = relationship(back_populates="scan", cascade=DELETE)
    snap: Mapped[Optional[Snap]] = relationship(back_populates="scan", cascade=DELETE)
    db: Mapped[DB] = relationship(back_populates="scans")

    @classmethod
    def create(cls, db: DB, multi_hits: bool, hmmer3_compat: bool, seqs: Iterable[Seq]):
        return cls(
            db=db,
            multi_hits=multi_hits,
            hmmer3_compat=hmmer3_compat,
            job=Job.create(type=JobType.scan),
            seqs=list(seqs),
        )

    def read_model(self):
        return ScanRead(
            id=self.id,
            job=self.job.read_model(),
            db=self.db.read_model(),
            multi_hits=self.multi_hits,
            hmmer3_compat=self.hmmer3_compat,
            seqs=[x.read_model() for x in self.seqs],
        )

    @staticmethod
    def get_by_id(session: Session, id: int):
        x = session.execute(select(Scan).where(Scan.id == id)).one_or_none()
        return x if x is None else x._tuple()[0]

    @staticmethod
    def get_all(session: Session, job_id: Optional[int] = None):
        clause = select(Scan)
        if job_id is not None:
            clause = clause.where(Scan.job_id == job_id)
        return [x._tuple()[0] for x in session.execute(clause).all()]
