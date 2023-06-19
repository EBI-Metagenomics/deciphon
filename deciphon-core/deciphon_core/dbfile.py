from pathlib import Path

from pydantic import BaseModel, FilePath, validator

__all__ = ["DBFile", "NewDBFile"]


class DBFile(BaseModel):
    path: FilePath

    @validator("path")
    def must_have_extension(cls, x: FilePath):
        if x.suffix != ".dcp":
            raise ValueError("must end in `.dcp`")
        return x


class NewDBFile(BaseModel):
    path: Path

    @validator("path")
    def must_have_extension(cls, x: Path):
        if x.suffix != ".dcp":
            raise ValueError("must end in `.dcp`")
        return x

    @validator("path")
    def must_not_exist(cls, x: Path):
        if x.exists():
            raise ValueError("path already exists")
        return x
