from pathlib import Path

from pydantic import BaseModel, FilePath, validator

__all__ = ["SnapFile", "NewSnapFile"]


class SnapFile(BaseModel):
    path: FilePath

    @validator("path")
    def must_have_extension(cls, x: FilePath):
        if x.suffix != ".dcs":
            raise ValueError("must end in `.dcs`")
        return x


class NewSnapFile(BaseModel):
    path: Path

    @validator("path")
    def must_have_extension(cls, x: Path):
        if x.suffix != ".dcs":
            raise ValueError("must end in `.dcs`")
        return x

    @validator("path")
    def must_not_exist(cls, x: Path):
        if x.exists():
            x.unlink()
        return x

    @validator("path")
    def basedir_must_not_exist(cls, x: Path):
        if basedir(x).exists():
            raise ValueError(f"`{basedir(x)}` must not exist")
        return x

    @property
    def basedir(self):
        return basedir(self.path)


def basedir(x: Path):
    return x.parent / f"{x.stem}"
