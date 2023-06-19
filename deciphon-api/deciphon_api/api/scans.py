from typing import List

from fastapi import APIRouter, Depends
from fastapi.responses import PlainTextResponse
from starlette.status import HTTP_200_OK, HTTP_201_CREATED, HTTP_204_NO_CONTENT

from deciphon_api.auth import auth_request
from deciphon_api.create_fasta import create_fasta
from deciphon_api.create_products import create_products
from deciphon_api.create_snap import create_snap as build_snap
from deciphon_api.errors import FileNotInStorageError
from deciphon_api.journal import get_journal
from deciphon_api.models import (
    DB,
    Job,
    JobType,
    Scan,
    ScanCreate,
    ScanRead,
    Seq,
    SeqRead,
    SnapCreate,
    SnapRead,
)
from deciphon_api.sched import get_sched, select
from deciphon_api.snap_gff import snap_gff
from deciphon_api.storage import storage_has
from deciphon_api.view_table import view_table

__all__ = ["router"]

router = APIRouter()

OK = HTTP_200_OK
NO_CONTENT = HTTP_204_NO_CONTENT
CREATED = HTTP_201_CREATED
AUTH = [Depends(auth_request)]
PLAIN = PlainTextResponse


@router.get("/scans", response_model=List[ScanRead], status_code=OK)
async def read_scans():
    with get_sched() as sched:
        return [ScanRead.from_orm(x) for x in sched.exec(select(Scan)).all()]


@router.post("/scans/", response_model=ScanRead, status_code=CREATED)
async def create_scan(scan: ScanCreate):
    with get_sched() as sched:
        sched.get(DB, scan.db_id)

        x = Scan.from_orm(scan)
        x.job = Job(type=JobType.scan)
        x.seqs = [Seq.from_orm(i) for i in scan.seqs]
        sched.add(x)
        sched.commit()
        sched.refresh(x)
        y = ScanRead.from_orm(x)
        await get_journal().publish_scan(y.id)
        return y


@router.get("/scans/{scan_id}", response_model=ScanRead, status_code=OK)
async def read_scan(scan_id: int):
    with get_sched() as sched:
        return ScanRead.from_orm(sched.get(Scan, scan_id))


@router.delete("/scans/{scan_id}", status_code=NO_CONTENT, dependencies=AUTH)
async def delete_scan(scan_id: int):
    with get_sched() as sched:
        sched.delete(sched.get(Scan, scan_id))
        sched.commit()


@router.get("/scans/{scan_id}/seqs", response_model=List[SeqRead], status_code=OK)
async def read_seqs(scan_id: int):
    with get_sched() as sched:
        scan = sched.get(Scan, scan_id)
        return [SeqRead.from_orm(i) for i in scan.seqs]


@router.put("/scans/{scan_id}/snap.dcs", response_model=SnapRead, status_code=CREATED)
async def create_snap(scan_id: int, snap: SnapCreate):
    if not storage_has(snap.sha256):
        raise FileNotInStorageError(snap.sha256)

    with get_sched() as sched:
        scan = sched.get(Scan, scan_id)
        scan.job.set_done()

        x = build_snap(scan_id, scan.seqs, snap)
        x.scan = scan
        sched.add(x)
        sched.commit()
        sched.refresh(x)

        prods = create_products(x)
        for prod in prods:
            sched.add(prod)
            sched.commit()
            sched.refresh(x)

        return SnapRead.from_orm(x)


@router.get("/scans/{scan_id}/snap.dcs", response_model=SnapRead, status_code=OK)
async def read_snap(scan_id: int):
    with get_sched() as sched:
        scan = sched.get(Scan, scan_id)
        return SnapRead.from_orm(scan.snap)


@router.get("/scans/{scan_id}/gff", response_class=PLAIN, status_code=OK)
async def read_gff(scan_id: int):
    with get_sched() as sched:
        scan = sched.get(Scan, scan_id)
        return snap_gff(scan.snap)


@router.get("/scans/{scan_id}/codon", response_class=PLAIN, status_code=OK)
async def read_codon(scan_id: int):
    with get_sched() as sched:
        scan = sched.get(Scan, scan_id)
        return create_fasta(scan.snap.prods, "codon")


@router.get("/scans/{scan_id}/amino", response_class=PLAIN, status_code=OK)
async def read_amino(scan_id: int):
    with get_sched() as sched:
        scan = sched.get(Scan, scan_id)
        return create_fasta(scan.snap.prods, "amino")


@router.get("/scans/{scan_id}/state", response_class=PLAIN, status_code=OK)
async def read_state(scan_id: int):
    with get_sched() as sched:
        scan = sched.get(Scan, scan_id)
        return create_fasta(scan.snap.prods, "state")


@router.get("/scans/{scan_id}/query", response_class=PLAIN, status_code=OK)
async def read_query(scan_id: int):
    with get_sched() as sched:
        scan = sched.get(Scan, scan_id)
        return create_fasta(scan.snap.prods, "query")


@router.get("/scans/{scan_id}/align", response_class=PLAIN, status_code=OK)
async def read_align(scan_id: int):
    with get_sched() as sched:
        scan = sched.get(Scan, scan_id)
        prods = scan.snap.prods
        return "\n".join(view_table(prod) for prod in prods)
