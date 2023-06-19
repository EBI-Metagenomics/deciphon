from __future__ import annotations

from sqlmodel import Field, SQLModel


class SeqBase(SQLModel):
    name: str = Field(nullable=False)
    data: str = Field(nullable=False)


class SeqCreate(SeqBase):
    ...


class SeqRead(SeqBase):
    id: int


class SeqUpdate(SQLModel):
    ...
