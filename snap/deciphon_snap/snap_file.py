from __future__ import annotations

from typing import List

import prettytable as pt
from deciphon_intervals import PyInterval
from h3result.read_h3result import read_h3result

from deciphon_snap.hmmer import H3Result
from deciphon_snap.match import LazyMatchList
from deciphon_snap.prod import Prod, ProdList
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
            rows = [stringify(x) for x in file]
            fieldnames = csv_fieldnames(rows[0])
            for idx, row in enumerate((csv_parse(fieldnames, r) for r in rows[1:])):
                seq_id = int(row["sequence"])
                profile = str(row["profile"])
                with fs.open(f"{hmmer_dir}/{seq_id}/{profile}.h3r", "rb") as f2:
                    h3r = H3Result(raw=read_h3result(fileno=f2.fileno()))
                window_start = int(row["window_start"])
                window_stop = int(row["window_stop"])
                hit_start = int(row["hit_start"])
                hit_stop = int(row["hit_stop"])
                window_interval = PyInterval(start=window_start, stop=window_stop)
                hit_interval = PyInterval(start=hit_start, stop=hit_stop)
                prods.append(
                    Prod(
                        id=idx,
                        seq_id=seq_id,
                        window=int(row["window"]),
                        window_interval=window_interval,
                        hit_interval=hit_interval,
                        profile=profile,
                        abc=row["abc"],
                        lrt=float(row["lrt"]),
                        evalue=float(row["evalue"]),
                        match_list=LazyMatchList(raw=str(row["match"])),
                        h3result=h3r,
                    )
                )
            self._prods = ProdList.model_validate(prods)

    @property
    def products(self):
        return self._prods

    def __str__(self):
        fields = Prod.model_fields.keys()

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


def csv_fieldnames(row: str):
    return row.strip().split("\t")


def csv_parse(fieldnames: list[str], row: str):
    return {name: field for name, field in zip(fieldnames, row.strip().split("\t"))}
