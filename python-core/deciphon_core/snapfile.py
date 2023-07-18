from pathlib import Path
import shutil

from pydantic import BaseModel, FilePath, field_validator

__all__ = ["SnapFile", "NewSnapFile"]


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
