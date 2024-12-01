from __future__ import annotations

import shutil
from enum import Enum, IntEnum
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
    "SnapName",
    "HMMPath",
    "DBPath",
    "SnapPath",
    "HMMFile",
    "DBFile",
    "SnapFile",
    "NewHMMFile",
    "NewDBFile",
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
    def dbname(self):
        return DBName(name=self.name[:-4] + ".dcp")


class DBName(BaseModel):
    name: str = Field(pattern=DB_NAME_PATTERN, max_length=NAME_MAX_LENGTH)

    @property
    def hmmname(self):
        return HMMName(name=self.name[:-4] + ".hmm")


class SnapName(BaseModel):
    name: str = Field(pattern=SNAP_NAME_PATTERN, max_length=NAME_MAX_LENGTH)


class HMMPath(BaseModel):
    path: Path

    @field_validator("path")
    def must_have_extension(cls, x: Path):
        if x.suffix != ".hmm":
            raise ValueError("must end in `.hmm`")
        return x

    @property
    def dbpath(self) -> DBPath:
        return DBPath(path=self.path.with_suffix(".dcp"))


class DBPath(BaseModel):
    path: Path

    @field_validator("path")
    def must_have_extension(cls, x: Path):
        if x.suffix != ".dcp":
            raise ValueError("must end in `.dcp`")
        return x

    @property
    def hmmpath(self) -> HMMPath:
        return HMMPath(path=self.path.with_suffix(".hmm"))


class SnapPath(BaseModel):
    path: Path

    @field_validator("path")
    def must_have_extension(cls, x: Path):
        if x.suffix != ".dcs":
            raise ValueError("must end in `.dcs`")
        return x


class HMMFile(BaseModel):
    path: FilePath

    @field_validator("path")
    def must_have_extension(cls, x: FilePath):
        if x.suffix != ".hmm":
            raise ValueError("must end in `.hmm`")
        return x

    @property
    def dbpath(self) -> DBPath:
        return DBPath(path=self.path.with_suffix(".dcp"))


class DBFile(BaseModel):
    path: FilePath

    @field_validator("path")
    def must_have_extension(cls, x: FilePath):
        if x.suffix != ".dcp":
            raise ValueError("must end in `.dcp`")
        return x

    @property
    def hmmpath(self) -> HMMPath:
        return HMMPath(path=self.path.with_suffix(".hmm"))


class SnapFile(BaseModel):
    path: FilePath

    @field_validator("path")
    def must_have_extension(cls, x: FilePath):
        if x.suffix != ".dcs":
            raise ValueError("must end in `.dcs`")
        return x


class NewHMMFile(BaseModel):
    path: Path

    @field_validator("path")
    def must_have_extension(cls, x: Path):
        if x.suffix != ".hmm":
            raise ValueError("must end in `.hmm`")
        return x

    @field_validator("path")
    def must_not_exist(cls, x: Path):
        if x.exists():
            raise ValueError("path already exists")
        return x

    @property
    def dbpath(self) -> DBPath:
        return DBPath(path=self.path.with_suffix(".dcp"))


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

    @property
    def hmmpath(self) -> HMMPath:
        return HMMPath(path=self.path.with_suffix(".hmm"))


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
        if x.with_suffix("").exists():
            y = x.with_suffix("")
            raise ValueError(f"`{y}` path must not exist")
        return x

    @property
    def basedir(self):
        return self.path.with_suffix("")

    def make_archive(self):
        basedir = self.basedir
        x = shutil.make_archive(str(basedir), "zip", self.path.parent, basedir.name)
        shutil.move(x, self.path)
        shutil.rmtree(basedir)
        return SnapFile(path=self.path)


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


class GencodeName(Enum):
    SGC0 = "The Standard Code"
    SGC1 = "The Vertebrate Mitochondrial Code"
    SGC2 = "The Yeast Mitochondrial Code"
    SGC3 = "The Mold, Protozoan, and Coelenterate Mitochondrial Code and the Mycoplasma/Spiroplasma Code"
    SGC4 = "The Invertebrate Mitochondrial Code"
    SGC5 = "The Ciliate, Dasycladacean and Hexamita Nuclear Code"
    SGC8 = "The Echinoderm and Flatworm Mitochondrial Code"
    SGC9 = "The Euplotid Nuclear Code"
    BAPP = "The Bacterial, Archaeal and Plant Plastid Code"
    AYN = "The Alternative Yeast Nuclear Code"
    AMC = "The Ascidian Mitochondrial Code"
    AFMC = "The Alternative Flatworm Mitochondrial Code"
    BMN = "Blepharisma Nuclear Code"
    CMC = "Chlorophycean Mitochondrial Code"
    TMC = "Trematode Mitochondrial Code"
    SOMC = "Scenedesmus obliquus Mitochondrial Code"
    TMMC = "Thraustochytrium Mitochondrial Code"
    PMMC = "Rhabdopleuridae Mitochondrial Code"
    CDSR1G = "Candidate Division SR1 and Gracilibacteria Code"
    PTN = "Pachysolen tannophilus Nuclear Code"
    KN = "Karyorelict Nuclear Code"
    CN = "Condylostoma Nuclear Code"
    MN = "Mesodinium Nuclear Code"
    PN = "Peritrich Nuclear Code"
    BN = "Blastocrithidia Nuclear Code"
    BP = "Balanophoraceae Plastid Code"
    CMMC = "Cephalodiscidae Mitochondrial UAA-Tyr Code"
