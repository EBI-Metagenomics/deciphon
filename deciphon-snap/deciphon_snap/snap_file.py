from __future__ import annotations

from csv import DictReader
from typing import List

import prettytable as pt
from h3result.read_h3result import read_h3result

from deciphon_snap.hmmer import H3Result
from deciphon_snap.match import LazyMatchList
from deciphon_snap.prod import Prod
from deciphon_snap.prod_list import ProdList
from deciphon_snap.shorten import shorten
from deciphon_snap.stringify import stringify

__all__ = ["SnapFile"]


class SnapFile:
    def __init__(self, filesystem):
        fs = filesystem

        files = fs.ls("/", detail=False)
        assert len(files) == 1

        root_dir = files[0].rstrip("/")
        assert fs.isdir(root_dir)
        prod_file = f"{root_dir}/products.tsv"

        hmmer_dir = f"{root_dir}/hmmer"
        assert fs.isdir(hmmer_dir)

        with fs.open(prod_file, "rb") as file:
            prods: List[Prod] = []
            reader = DictReader((stringify(x) for x in file), delimiter="\t")
            for idx, row in enumerate(reader):
                seq_id = int(row["seq_id"])
                profile = str(row["profile"])
                with fs.open(f"{hmmer_dir}/{seq_id}/{profile}.h3r", "rb") as f2:
                    h3r = H3Result(raw=read_h3result(fileno=f2.fileno()))
                prods.append(
                    Prod(
                        id=idx,
                        seq_id=seq_id,
                        profile=profile,
                        abc=str(row["abc"]),
                        alt=float(row["alt"]),
                        null=float(row["null"]),
                        evalue=float(row["evalue"]),
                        match_list=LazyMatchList(raw=str(row["match"])),
                        h3result=h3r,
                    )
                )
            self._prods = ProdList.parse_obj(prods)

    @property
    def products(self):
        return self._prods

    def __str__(self):
        fields = Prod.__fields__

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
