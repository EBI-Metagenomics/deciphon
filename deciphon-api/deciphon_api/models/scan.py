from __future__ import annotations

from typing import List

from sqlmodel import Field, SQLModel

from deciphon_api.models.seq import SeqCreate, SeqRead


class ScanBase(SQLModel):
    multi_hits: bool = Field(nullable=False)
    hmmer3_compat: bool = Field(nullable=False)
    db_id: int = Field(foreign_key="db.id")


class ScanCreate(ScanBase):
    seqs: List[SeqCreate]


class ScanRead(ScanBase):
    id: int
    seqs: List[SeqRead]


class ScanUpdate(SQLModel):
    ...
