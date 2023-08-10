from pydantic import (
    AfterValidator,
    BaseModel,
)
from typing_extensions import Annotated

__all__ = ["gencode", "Gencode"]


def check_ncbi_genetic_code(x):
    assert x in NCBI_GENETIC_CODES, f"must be one of {NCBI_GENETIC_CODES}"
    return x


gencode = Annotated[int, AfterValidator(check_ncbi_genetic_code)]


class Gencode(BaseModel):
    id: gencode


NCBI_GENETIC_CODES = [
    1,
    2,
    3,
    4,
    5,
    6,
    9,
    10,
    11,
    12,
    13,
    14,
    15,
    16,
    21,
    22,
    23,
    24,
    25,
    26,
    27,
    28,
    29,
    30,
    31,
    32,
    33,
]

if __name__ == "__main__":
    print(Gencode(id=1))
