from fastapi import APIRouter, Request
from starlette.status import HTTP_200_OK, HTTP_201_CREATED, HTTP_204_NO_CONTENT

from ..database import Database
from ..errors import NotFoundInDatabaseError
from ..journal import Journal
from .models import DB, Scan, Seq
from .schemas import ScanCreate, ScanRead

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
    await journal.publish("scans", scan_read.model_dump_json())

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
