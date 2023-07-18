from __future__ import annotations

from enum import Enum

from pydantic import BaseModel


class StreamName(str, Enum):
    AMINO = "amino"
    CODON = "codon"
    QUERY = "query"
    STATE = "state"
    H3HMM_CS = "h3hmm_cs"
    H3QUERY_CS = "h3query_cs"
    H3MATCH = "h3match"
    H3QUERY = "h3query"
    H3SCORE = "h3score"


class Stream(BaseModel):
    name: StreamName
    level: int | None = None
