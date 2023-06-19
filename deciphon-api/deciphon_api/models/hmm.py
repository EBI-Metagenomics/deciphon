from __future__ import annotations

from sqlmodel import Field, SQLModel

from deciphon_api.filename import filename_regex
from deciphon_api.sha256 import SHA256_REGEX


class HMMBase(SQLModel):
    sha256: str = Field(index=True, unique=True, regex=SHA256_REGEX)
    filename: str = Field(index=True, unique=True, regex=filename_regex("hmm"))


class HMMCreate(HMMBase):
    ...


class HMMRead(HMMBase):
    id: int
    job_id: int
