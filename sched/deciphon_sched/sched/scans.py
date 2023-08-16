import tempfile
from typing import Annotated

from deciphon_snap.read_snap import read_snap
from fastapi import APIRouter, File, Request
from starlette.status import HTTP_200_OK, HTTP_201_CREATED, HTTP_204_NO_CONTENT

from deciphon_sched.database import Database
from deciphon_sched.errors import (
    FoundInDatabaseError,
    NotFoundInDatabaseError,
    SnapFileError,
)
from deciphon_sched.journal import Journal
from deciphon_sched.sched.models import DB, Scan, Seq, Snap
from deciphon_sched.sched.schemas import ScanCreate, ScanRead, ScanRequest

router = APIRouter()


@router.get("/scans", status_code=HTTP_200_OK)
async def read_scans(request: Request) -> list[ScanRead]:
    database: Database = request.app.state.database
    with database.create_session() as session:
        return [x.read_model() for x in Scan.get_all(session)]


@router.post("/scans/", status_code=HTTP_201_CREATED)
async def create_scan(request: Request, scan: ScanCreate) -> ScanRead:
    database: Database = request.app.state.database
    with database.create_session() as session:
        db = DB.get_by_id(session, scan.db_id)
        if db is None:
            raise NotFoundInDatabaseError("DB")

        seqs = [Seq.create(x.name, x.data) for x in scan.seqs]
        x = Scan.create(db, scan.multi_hits, scan.hmmer3_compat, seqs)
        for seq in seqs:
            seq.scan = x
        session.add_all([x] + seqs)
        session.commit()
        scan_read = x.read_model()

    journal: Journal = request.app.state.journal
    x = ScanRequest.create(scan_read)
    await journal.publish("scan", x.model_dump_json())

    return scan_read


@router.get("/scans/{scan_id}", status_code=HTTP_200_OK)
async def read_scan(request: Request, scan_id: int) -> ScanRead:
    database: Database = request.app.state.database
    with database.create_session() as session:
        x = Scan.get_by_id(session, scan_id)
        if x is None:
            raise NotFoundInDatabaseError("Scan")
        return x.read_model()


@router.delete("/scans/{scan_id}", status_code=HTTP_204_NO_CONTENT)
async def delete_scan(request: Request, scan_id: int):
    database: Database = request.app.state.database
    with database.create_session() as session:
        x = Scan.get_by_id(session, scan_id)
        if x is None:
            raise NotFoundInDatabaseError("Scan")
        session.delete(x)
        session.commit()


@router.post("/scans/{scan_id}.dcs", status_code=HTTP_201_CREATED)
async def create_snap(request: Request, scan_id: int, file: Annotated[bytes, File()]):
    with tempfile.NamedTemporaryFile() as tmp:
        tmp.write(file)
        tmp.flush()
        try:
            read_snap(tmp.name)
        except Exception as exception:
            raise SnapFileError(str(exception))

    database: Database = request.app.state.database
    with database.create_session() as session:
        x = Snap.get_by_id(session, scan_id)
        if x is not None:
            raise FoundInDatabaseError("Snap")
        snap = Snap.create(scan_id, file)
        session.add(snap)
        session.commit()
        return snap.read_model()
