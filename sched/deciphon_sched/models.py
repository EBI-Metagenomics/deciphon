from __future__ import annotations
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


class Job(Base):
    __tablename__ = "job"

    id: Mapped[int] = mapped_column(primary_key=True)

    hmm: Mapped[Optional[HMM]] = relationship(back_populates="job")
    scan: Mapped[Optional[Scan]] = relationship(back_populates="job")


class HMM(Base):
    __tablename__ = "hmm"

    id: Mapped[int] = mapped_column(primary_key=True)
    job_id: Mapped[Optional[int]] = mapped_column(ForeignKey("job.id"))

    job: Mapped[Job] = relationship(back_populates="hmm")
    db: Mapped[Optional[DB]] = relationship(back_populates="hmm")


class DB(Base):
    __tablename__ = "db"

    id: Mapped[int] = mapped_column(primary_key=True)
    hmm_id: Mapped[int] = mapped_column(ForeignKey("hmm.id"))

    hmm: Mapped[HMM] = relationship(back_populates="db")
    scans: Mapped[list[Scan]] = relationship(back_populates="db")


class Scan(Base):
    __tablename__ = "scan"

    id: Mapped[int] = mapped_column(primary_key=True)
    job_id: Mapped[int] = mapped_column(ForeignKey("job.id"))
    db_id: Mapped[int] = mapped_column(ForeignKey("db.id"))

    job: Mapped[Job] = relationship(back_populates="scan")
    db: Mapped[DB] = relationship(back_populates="scans")
