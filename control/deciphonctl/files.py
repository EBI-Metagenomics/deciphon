from __future__ import annotations
import os
import re
import shutil
import uuid
from contextlib import contextmanager
from pathlib import Path

__all__ = ["atomic_file_creation", "unique_temporary_file", "remove_temporary_files"]

_HEXSIZE = 16
_HEXCHAR = "[abcdef0-9]"
_REGEX = r".*\." + _HEXCHAR + "{" + str(_HEXSIZE) + "}" + r"tmp\..*"


@contextmanager
def atomic_file_creation(path: Path):
    hex = str(uuid.uuid4().hex)[:_HEXSIZE]
    tmp = path.with_suffix(f".{hex}tmp{path.suffix}")
    try:
        yield tmp
        shutil.move(tmp, path)
    finally:
        tmp.unlink(missing_ok=True)


@contextmanager
def unique_temporary_file(suffix: str):
    assert len(suffix) > 1 and suffix[0] == "." and suffix[1] != "."
    hex = str(uuid.uuid4().hex)[:_HEXSIZE]
    tmp = Path(f".{hex}tmp{suffix}")
    try:
        yield tmp
    finally:
        tmp.unlink(missing_ok=True)


def remove_temporary_files():
    for f in _glob_re(_REGEX, os.listdir()):
        Path(f).unlink(missing_ok=True)


def _glob_re(pattern, strings):
    return filter(re.compile(pattern).match, strings)
