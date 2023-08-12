import typer
from deciphon_core.schema import Gencode
from typing_extensions import Annotated


def parse_gencode(value: str):
    return Gencode(int(value))


GENCODE = Annotated[
    Gencode, typer.Argument(parser=parse_gencode, help="NCBI genetic code number")
]
EPSILON = Annotated[float, typer.Argument(help="Nucleotide error probability")]
