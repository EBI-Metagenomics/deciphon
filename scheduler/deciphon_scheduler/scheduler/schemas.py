from enum import Enum

from pydantic import BaseModel, Field


class JobType(Enum):
    hmm = "hmm"
    scan = "scan"


class JobState(Enum):
    pend = "pend"
    run = "run"
    done = "done"
    fail = "fail"


def _file_name_pattern(ext: str):
    return r"^[0-9a-zA-Z_\-.][0-9a-zA-Z_\-. ]+\." + ext + "$"


_SHA256_PATTERN = r"^[0123456789abcdef]{64}$"


class HMMFile(BaseModel):
    name: str = Field(pattern=_file_name_pattern("hmm"))
    sha256: str = Field(pattern=_SHA256_PATTERN)


class DBFile(BaseModel):
    name: str = Field(pattern=_file_name_pattern("dcp"))
    sha256: str = Field(pattern=_SHA256_PATTERN)


class SnapFile(BaseModel):
    name: str = Field(pattern=_file_name_pattern("dcs"))
    sha256: str = Field(pattern=_SHA256_PATTERN)


class HMMRead(BaseModel):
    id: int
    job_id: int
    file: HMMFile
