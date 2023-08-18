from __future__ import annotations

import shutil
from enum import IntEnum
from pathlib import Path

from pydantic import (
    BaseModel,
    Field,
    FilePath,
    field_validator,
)

__all__ = [
    "HMMName",
    "DBName",
    "HMMFile",
    "DBFile",
    "NewDBFile",
    "SnapName",
    "SnapFile",
    "NewSnapFile",
    "Gencode",
    "NAME_MAX_LENGTH",
    "HMM_NAME_PATTERN",
    "DB_NAME_PATTERN",
    "SNAP_NAME_PATTERN",
]


def _file_name_pattern(ext: str):
    return r"^[0-9a-zA-Z_\-.][0-9a-zA-Z_\-. ]+\." + ext + "$"


NAME_MAX_LENGTH = 128

HMM_NAME_PATTERN = _file_name_pattern("hmm")
DB_NAME_PATTERN = _file_name_pattern("dcp")
SNAP_NAME_PATTERN = _file_name_pattern("dcs")


class HMMName(BaseModel):
    name: str = Field(pattern=HMM_NAME_PATTERN, max_length=NAME_MAX_LENGTH)

    @property
    def db_name(self):
        return DBName(name=self.name[:-4] + ".dcp")


class DBName(BaseModel):
    name: str = Field(pattern=DB_NAME_PATTERN, max_length=NAME_MAX_LENGTH)

    @property
    def hmm_file_name(self):
        return HMMName(name=self.name[:-4] + ".hmm")


class SnapName(BaseModel):
    name: str = Field(pattern=SNAP_NAME_PATTERN, max_length=NAME_MAX_LENGTH)


class HMMFile(BaseModel):
    path: FilePath

    @field_validator("path")
    def must_have_extension(cls, x: FilePath):
        if x.suffix != ".hmm":
            raise ValueError("must end in `.hmm`")
        return x

    @property
    def _dbpath(self) -> Path:
        return self.path.parent / f"{self.path.stem}.dcp"

    @property
    def dbfile(self) -> DBFile:
        return DBFile(path=self._dbpath)

    @property
    def newdbfile(self) -> NewDBFile:
        return NewDBFile(path=self._dbpath)


class DBFile(BaseModel):
    path: FilePath

    @field_validator("path")
    def must_have_extension(cls, x: FilePath):
        if x.suffix != ".dcp":
            raise ValueError("must end in `.dcp`")
        return x


class NewDBFile(BaseModel):
    path: Path

    @field_validator("path")
    def must_have_extension(cls, x: Path):
        if x.suffix != ".dcp":
            raise ValueError("must end in `.dcp`")
        return x

    @field_validator("path")
    def must_not_exist(cls, x: Path):
        if x.exists():
            raise ValueError("path already exists")
        return x


class SnapFile(BaseModel):
    path: FilePath

    @field_validator("path")
    def must_have_extension(cls, x: FilePath):
        if x.suffix != ".dcs":
            raise ValueError("must end in `.dcs`")
        return x


class NewSnapFile(BaseModel):
    path: Path

    @field_validator("path")
    def must_have_extension(cls, x: Path):
        if x.suffix != ".dcs":
            raise ValueError("must end in `.dcs`")
        return x

    @field_validator("path")
    def must_not_exist(cls, x: Path):
        if x.exists():
            x.unlink()
        return x

    @field_validator("path")
    def basedir_must_not_exist(cls, x: Path):
        if basedir(x).exists():
            raise ValueError(f"`{basedir(x)}` must not exist")
        return x

    @property
    def basename(self):
        return basedir(self.path)

    def make_archive(self):
        basename = self.basename
        x = shutil.make_archive(str(basename), "zip", self.path.parent, basename.name)
        shutil.move(x, self.path)
        shutil.rmtree(basename)


def basedir(x: Path):
    return x.parent / str(x.stem)


class Gencode(IntEnum):
    """NCBI genetic codes."""

    SGC0 = 1
    SGC1 = 2
    SGC2 = 3
    SGC3 = 4
    SGC4 = 5
    SGC5 = 6
    SGC8 = 9
    SGC9 = 10
    BAPP = 11
    AYN = 12
    AMC = 13
    AFMC = 14
    BMN = 15
    CMC = 16
    TMC = 21
    SOMC = 22
    TMMC = 23
    PMMC = 24
    CDSR1G = 25
    PTN = 26
    KN = 27
    CN = 28
    MN = 29
    PN = 30
    BN = 31
    BP = 32
    CMMC = 33

    # IntEnum of Python3.10 returns a different string representation.
    # Make it return the same as in Python3.11
    def __str__(self):
        return str(self.value)
