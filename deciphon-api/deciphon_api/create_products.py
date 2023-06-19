from deciphon_api.models import Prod, Snap
from deciphon_api.read_products import read_products
from deciphon_api.snap_fs import snap_fs

__all__ = ["create_products"]


def create_products(snap: Snap):
    fs = snap_fs(snap.sha256)
    with fs.open("snap/products.tsv", "rb") as file:
        prods = read_products(file)
        return [Prod.from_orm(i, update={"snap_id": snap.id}) for i in prods]
