import csv
from typing import List, Union

from deciphon_api.models import ProdCreate

__all__ = ["read_products"]


def stringify(x: Union[bytes, str]):
    if isinstance(x, bytes):
        return x.decode()
    return x


def read_products(file):
    prods: List[ProdCreate] = []
    for row in csv.DictReader((stringify(i) for i in file), delimiter="\t"):
        prods.append(ProdCreate.parse_obj(row))
    return prods
