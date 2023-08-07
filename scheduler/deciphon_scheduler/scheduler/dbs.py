from typing import Annotated
from deciphon_scheduler.storage import PresignedUpload, Storage
from fastapi import APIRouter, Request, Path
from starlette.status import HTTP_200_OK, HTTP_201_CREATED, HTTP_204_NO_CONTENT
from deciphon_scheduler.scheduler.validation import (
    FILE_NAME_MAX_LENGTH,
    DB_FILE_NAME_PATTERN,
)

from deciphon_scheduler.database import Database
from deciphon_scheduler.errors import (
    FileNameExistsError,
    FileNameNotFoundError,
    NotFoundInDatabaseError,
)
from deciphon_scheduler.journal import Journal
from deciphon_scheduler.scheduler.models import DB, HMM
from deciphon_scheduler.scheduler.schemas import DBFileName, DBRead

router = APIRouter()


@router.get("/dbs", status_code=HTTP_200_OK)
async def read_dbs(request: Request) -> list[DBRead]:
    database: Database = request.app.state.database
    with database.create_session() as session:
        return [x.read_model() for x in DB.get_all(session)]


@router.get("/dbs/presigned-upload/{name}", status_code=HTTP_200_OK)
async def presigned_db_upload(
    request: Request,
    name: Annotated[
        str,
        Path(
            title="DB file name",
            pattern=DB_FILE_NAME_PATTERN,
            max_length=FILE_NAME_MAX_LENGTH,
        ),
    ],
) -> PresignedUpload:
    storage: Storage = request.app.state.storage
    database: Database = request.app.state.database

    db = DBFileName(name=name)
    with database.create_session() as session:
        x = DB.get_by_file_name(session, db.name)
        if x is not None:
            raise FileNameExistsError(db.name)
        if storage.has_file(db.name):
            raise FileNameExistsError(db.name)
        return storage.presigned_upload(db.name)


@router.post("/dbs/", status_code=HTTP_201_CREATED)
async def create_db(request: Request, db: DBFileName) -> DBRead:
    storage: Storage = request.app.state.storage
    database: Database = request.app.state.database

    with database.create_session() as session:
        if DB.get_by_file_name(session, db.name) is not None:
            raise FileNameExistsError(db.name)

        hmm = HMM.get_by_file_name(session, db.hmm_file_name.name)
        if hmm is None:
            raise FileNameNotFoundError(db.hmm_file_name.name)

        if not storage.has_file(db.name):
            raise FileNameNotFoundError(db.name)

        hmm.job.set_done()

        x = DB.create(hmm, db)
        session.add(x)
        session.commit()
        db_read = x.read_model()

    journal: Journal = request.app.state.journal
    await journal.publish("dbs", db_read.model_dump_json())

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
        if storage.has_file(x.file_name):
            storage.delete(x.file_name)
        session.delete(x)
        session.commit()
