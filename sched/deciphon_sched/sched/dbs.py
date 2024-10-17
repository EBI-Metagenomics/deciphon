from typing import Annotated

from deciphon_core.schema import DBName
from fastapi import APIRouter, Request
from starlette.status import HTTP_200_OK, HTTP_201_CREATED, HTTP_204_NO_CONTENT

from deciphon_sched.database import Database
from deciphon_sched.errors import (
    FileNameExistsError,
    FileNameNotFoundError,
    NotFoundInDatabaseError,
)
from deciphon_sched.sched.models import DB, HMM
from deciphon_sched.sched.schemas import DBFilePathType, DBRead
from deciphon_sched.storage import PresignedDownload, PresignedUpload, Storage

router = APIRouter()


@router.get("/dbs", status_code=HTTP_200_OK)
async def read_dbs(request: Request) -> list[DBRead]:
    database: Database = request.app.state.database
    with database.create_session() as session:
        return [x.read_model() for x in DB.get_all(session)]


@router.get("/dbs/presigned-upload/{name}", status_code=HTTP_200_OK)
async def presigned_db_upload(
    request: Request,
    name: Annotated[str, DBFilePathType],
) -> PresignedUpload:
    storage: Storage = request.app.state.storage
    database: Database = request.app.state.database

    with database.create_session() as session:
        x = DB.get_by_name(session, name)
        if x is not None:
            raise FileNameExistsError(name)
        return storage.presigned_upload(name)


@router.get("/dbs/presigned-download/{name}", status_code=HTTP_200_OK)
async def presigned_db_download(
    request: Request,
    name: Annotated[str, DBFilePathType],
) -> PresignedDownload:
    storage: Storage = request.app.state.storage
    database: Database = request.app.state.database

    with database.create_session() as session:
        x = DB.get_by_name(session, name)
        if x is None:
            raise FileNameNotFoundError(name)
        return storage.presigned_download(name)


@router.post("/dbs/", status_code=HTTP_201_CREATED)
async def create_db(request: Request, db: DBName) -> DBRead:
    storage: Storage = request.app.state.storage
    database: Database = request.app.state.database

    with database.create_session() as session:
        if DB.get_by_name(session, db.name) is not None:
            raise FileNameExistsError(db.name)

        hmm = HMM.get_by_name(session, db.hmm_name.name)
        if hmm is None:
            raise FileNameNotFoundError(db.hmm_name.name)

        if not storage.has_file(db.name):
            raise FileNameNotFoundError(db.name)

        hmm.job.set_done()

        x = DB.create(hmm, db)
        session.add(x)
        session.commit()
        db_read = x.read_model()

    return db_read


@router.get("/dbs/{db_id}", status_code=HTTP_200_OK)
async def read_db(request: Request, db_id: int) -> DBRead:
    database: Database = request.app.state.database
    with database.create_session() as session:
        x = DB.get_by_id(session, db_id)
        if x is None:
            raise NotFoundInDatabaseError("DB")
        return x.read_model()


@router.delete("/dbs/{db_id}", status_code=HTTP_204_NO_CONTENT)
async def delete_db(request: Request, db_id: int):
    storage: Storage = request.app.state.storage
    database: Database = request.app.state.database

    with database.create_session() as session:
        x = DB.get_by_id(session, db_id)
        if x is None:
            raise NotFoundInDatabaseError("DB")
        if storage.has_file(x.name):
            storage.delete(x.name)
        session.delete(x)
        session.commit()
