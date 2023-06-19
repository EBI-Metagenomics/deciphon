from __future__ import annotations

from sqlmodel import Field, SQLModel

from deciphon_api.filename import filename_regex
from deciphon_api.sha256 import SHA256_REGEX


class DBBase(SQLModel):
    sha256: str = Field(index=True, unique=True, regex=SHA256_REGEX)
    filename: str = Field(index=True, unique=True, regex=filename_regex("dcp"))

    @property
    def expected_hmm_filename(self) -> str:
        return self.filename[:-4] + ".hmm"


class DBCreate(DBBase):
    ...


class DBRead(DBBase):
    id: int
    hmm_id: int
