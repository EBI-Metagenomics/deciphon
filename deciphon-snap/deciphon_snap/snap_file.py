from __future__ import annotations

from csv import DictReader
from typing import List

import prettytable as pt

from deciphon_snap.match_list import MatchList
from deciphon_snap.prod import Prod
from deciphon_snap.prod_list import ProdList
from deciphon_snap.shorten import shorten
from deciphon_snap.stringify import stringify

__all__ = ["SnapFile"]


class SnapFile:
    def __init__(self, filesystem):
        self._fs = filesystem

        files = self._fs.ls("/")
        assert len(files) == 1

        root_dir = files[0]["filename"][:-1]
        assert self._fs.isdir(root_dir)
        prod_file = f"{root_dir}/products.tsv"

        with self._fs.open(prod_file, "rb") as file:
            self._prods = read_products(file)

    @property
    def products(self):
        return self._prods

    def __str__(self):
        fields = list(Prod.schema()["properties"].keys())

        num_fields = len(fields)
        prods = [[getattr(x, i) for i in fields] for x in self.products]
        num_products = len(prods)
        if num_products >= 10:
            prods = prods[:4] + [["…"] * num_fields] + prods[-4:]

        x = pt.PrettyTable()
        x.set_style(pt.SINGLE_BORDER)
        x.field_names = fields
        x.align = "l"
        for prod in prods:
            x.add_row([shorten(i) for i in prod])

        header = f"shape: ({num_products}, {num_fields})"
        return header + "\n" + x.get_string()


def read_products(file):
    prods: List[Prod] = []
    for i, row in enumerate(DictReader((stringify(i) for i in file), delimiter="\t")):
        prods.append(
            Prod(
                id=i,
                seq_id=int(row["seq_id"]),
                profile=str(row["profile"]),
                abc=str(row["abc"]),
                alt=float(row["alt"]),
                null=float(row["null"]),
                evalue=float(row["evalue"]),
                match_list=MatchList.from_string(str(row["match"])),
            )
        )
    return ProdList.parse_obj(prods)
