from typing import Annotated

from fastapi import APIRouter, Path, Request
from starlette.status import HTTP_200_OK, HTTP_201_CREATED, HTTP_204_NO_CONTENT

from deciphon_scheduler.scheduler.validation import (
    FILE_NAME_MAX_LENGTH,
    HMM_FILE_NAME_PATTERN,
)

from deciphon_scheduler.database import Database
from deciphon_scheduler.errors import (
    FileNameExistsError,
    FileNameNotFoundError,
    NotFoundInDatabaseError,
)
from deciphon_scheduler.journal import Journal
from deciphon_scheduler.storage import PresignedUpload, Storage
from deciphon_scheduler.scheduler.models import HMM
from deciphon_scheduler.scheduler.schemas import HMMFileName, HMMRead

router = APIRouter()


@router.get("/hmms", status_code=HTTP_200_OK)
async def read_hmms(request: Request) -> list[HMMRead]:
    database: Database = request.app.state.database
    with database.create_session() as session:
        return [x.read_model() for x in HMM.get_all(session)]


@router.get("/hmms/presigned-upload/{file_name}", status_code=HTTP_200_OK)
async def presigned_hmm_upload(
    request: Request,
    file_name: Annotated[
        str,
        Path(
            title="HMM file name",
            pattern=HMM_FILE_NAME_PATTERN,
            max_length=FILE_NAME_MAX_LENGTH,
        ),
    ],
) -> PresignedUpload:
    storage: Storage = request.app.state.storage
    database: Database = request.app.state.database

    hmm = HMMFileName(name=file_name)
    with database.create_session() as session:
        x = HMM.get_by_file_name(session, hmm.name)
        if x is not None:
            raise FileNameExistsError(hmm.name)
        if storage.has_file(hmm.name):
            raise FileNameExistsError(hmm.name)
        return storage.presigned_upload(hmm.name)


@router.post("/hmms/", status_code=HTTP_201_CREATED)
async def create_hmm(request: Request, hmm: HMMFileName) -> HMMRead:
    storage: Storage = request.app.state.storage
    database: Database = request.app.state.database

    with database.create_session() as session:
        x = HMM.get_by_file_name(session, hmm.name)
        if x is not None:
            raise FileNameExistsError(hmm.name)

        if not storage.has_file(hmm.name):
            FileNameNotFoundError(hmm.name)

        x = HMM.create(hmm)
        session.add(x)
        session.commit()
        hmm_read = x.read_model()

    journal: Journal = request.app.state.journal
    await journal.publish("hmms", hmm_read.model_dump_json())

    return hmm_read


@router.get("/hmms/{hmm_id}", status_code=HTTP_200_OK)
async def read_hmm(request: Request, hmm_id: int) -> HMMRead:
    database: Database = request.app.state.database
    with database.create_session() as session:
        x = HMM.get_by_id(session, hmm_id)
        if x is None:
            raise NotFoundInDatabaseError("HMM")
        return x.read_model()


@router.delete("/hmms/{hmm_id}", status_code=HTTP_204_NO_CONTENT)
async def delete_hmm(request: Request, hmm_id: int):
    database: Database = request.app.state.database
    with database.create_session() as session:
        x = HMM.get_by_id(session, hmm_id)
        if x is None:
            raise NotFoundInDatabaseError("HMM")
        session.delete(x)
        session.commit()
