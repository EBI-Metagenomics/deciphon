from typing import List

from fastapi import APIRouter, Depends
from starlette.status import HTTP_200_OK, HTTP_201_CREATED, HTTP_204_NO_CONTENT

from deciphon_api.auth import auth_request
from deciphon_api.errors import FileNotInStorageError
from deciphon_api.models import (
    DB,
    HMM,
    DBCreate,
    DBRead,
)
from deciphon_api.sched import get_sched, select
from deciphon_api.storage import storage_has

__all__ = ["router"]

router = APIRouter()

OK = HTTP_200_OK
NO_CONTENT = HTTP_204_NO_CONTENT
CREATED = HTTP_201_CREATED
AUTH = [Depends(auth_request)]


@router.get("/dbs", response_model=List[DBRead], status_code=OK)
async def read_dbs():
    with get_sched() as sched:
        return [DBRead.from_orm(x) for x in sched.exec(select(DB)).all()]


@router.post("/dbs/", response_model=DBRead, status_code=CREATED, dependencies=AUTH)
async def create_db(db: DBCreate):
    if not storage_has(db.sha256):
        raise FileNotInStorageError(db.sha256)

    with get_sched() as sched:
        stmt = select(HMM).where(HMM.filename == db.expected_hmm_filename)
        hmm = sched.exec(stmt).one_or_none()
        hmm.job.set_done()

        x = DB.from_orm(db)
        x.hmm = hmm
        sched.add(x)
        sched.commit()
        sched.refresh(x)
        return DBRead.from_orm(x)


@router.get("/dbs/{db_id}", response_model=DBRead, status_code=OK)
async def read_db(db_id: int):
    with get_sched() as sched:
        db = sched.get(DB, db_id)
        return DBRead.from_orm(db)


@router.delete("/dbs/{db_id}", status_code=NO_CONTENT, dependencies=AUTH)
async def delete_db(db_id: int):
    with get_sched() as sched:
        db = sched.get(DB, db_id)
        sched.delete(db)
        sched.commit()
