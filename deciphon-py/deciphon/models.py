from typing import List

from pydantic import BaseModel


class HMM(BaseModel):
    id: int
    filename: str
    sha256: str
    job_id: int


class HMMCreate(BaseModel):
    sha256: str
    filename: str


class DB(BaseModel):
    id: int
    filename: str
    sha256: str
    hmm_id: int


class DBCreate(BaseModel):
    sha256: str
    filename: str


class Seq(BaseModel):
    id: int
    name: str
    data: str


class SeqCreate(BaseModel):
    name: str
    data: str


class Scan(BaseModel):
    id: int
    multi_hits: bool
    hmmer3_compat: bool
    db_id: int
    seqs: List[Seq]


class ScanCreate(BaseModel):
    multi_hits: bool
    hmmer3_compat: bool
    db_id: int
    seqs: List[SeqCreate]


class SnapCreate(BaseModel):
    sha256: str
    filename: str
