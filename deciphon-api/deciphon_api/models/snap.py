from __future__ import annotations

from sqlmodel import Field, SQLModel

from deciphon_api.filename import filename_regex
from deciphon_api.sha256 import SHA256_REGEX


class SnapBase(SQLModel):
    scan_id: int = Field(foreign_key="scan.id")

    sha256: str = Field(regex=SHA256_REGEX)
    filename: str = Field(regex=filename_regex("dcs"))


class SnapCreate(SQLModel):
    sha256: str = Field(regex=SHA256_REGEX)
    filename: str = Field(regex=r"^snap\.dcs$")


class SnapRead(SnapBase):
    ...


class SnapUpdate(SQLModel):
    ...
