from pathlib import Path

from pydantic import BaseModel, FilePath, validator

from deciphon_core.dbfile import DBFile, NewDBFile

__all__ = ["HMMFile"]


class HMMFile(BaseModel):
    path: FilePath

    @validator("path")
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
