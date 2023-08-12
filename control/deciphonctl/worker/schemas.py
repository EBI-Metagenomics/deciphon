from enum import Enum

from deciphon_core.schema import DBName, Gencode, HMMName
from pydantic import BaseModel


class HMMFile(HMMName):
    ...


class DBFile(DBName):
    gencode: Gencode
    epsilon: float


class PressRequest(BaseModel):
    job_id: int
    hmm: HMMFile
    db: DBFile


class Task(Enum):
    press = "press"
    scan = "scan"


class JobState(Enum):
    pend = "pend"
    run = "run"
    done = "done"
    fail = "fail"


class JobUpdate(BaseModel):
    id: int
    state: JobState
    progress: int
    error: str
