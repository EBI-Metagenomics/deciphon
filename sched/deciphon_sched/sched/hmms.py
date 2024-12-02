from typing import Annotated

from deciphon_schema import Gencode, HMMName
from fastapi import APIRouter, Request
from starlette.status import HTTP_200_OK, HTTP_201_CREATED, HTTP_204_NO_CONTENT

from deciphon_sched.database import Database
from deciphon_sched.errors import (
    FileNameExistsError,
    FileNameNotFoundError,
    NotFoundInDatabaseError,
)
from deciphon_sched.journal import Journal
from deciphon_sched.sched.models import HMM
from deciphon_sched.sched.schemas import HMMFilePathType, HMMRead, PressRequest
from deciphon_sched.storage import PresignedDownload, PresignedUpload, Storage

router = APIRouter()


@router.get("/hmms", status_code=HTTP_200_OK)
async def read_hmms(request: Request) -> list[HMMRead]:
    database: Database = request.app.state.database
    with database.create_session() as session:
        return [x.read_model() for x in HMM.get_all(session)]


@router.get("/hmms/presigned-upload/{name}", status_code=HTTP_200_OK)
async def presigned_hmm_upload(
    request: Request,
    name: Annotated[str, HMMFilePathType],
) -> PresignedUpload:
    storage: Storage = request.app.state.storage
    database: Database = request.app.state.database

    with database.create_session() as session:
        x = HMM.get_by_name(session, HMMName(name=name))
        if x is not None:
            raise FileNameExistsError(name)
        return storage.presigned_upload(name)


@router.get("/hmms/presigned-download/{name}", status_code=HTTP_200_OK)
async def presigned_hmm_download(
    request: Request,
    name: Annotated[str, HMMFilePathType],
) -> PresignedDownload:
    storage: Storage = request.app.state.storage
    database: Database = request.app.state.database

    with database.create_session() as session:
        x = HMM.get_by_name(session, HMMName(name=name))
        if x is None:
            raise FileNameNotFoundError(name)
        return storage.presigned_download(name)


@router.post("/hmms/", status_code=HTTP_201_CREATED)
async def create_hmm(
    request: Request, hmm: HMMName, gencode: Gencode, epsilon: float
) -> HMMRead:
    storage: Storage = request.app.state.storage
    database: Database = request.app.state.database

    with database.create_session() as session:
        x = HMM.get_by_name(session, hmm)
        if x is not None:
            raise FileNameExistsError(hmm.name)

        if not storage.has_file(hmm.name):
            raise FileNameNotFoundError(hmm.name)

        x = HMM.create(hmm)
        session.add(x)
        session.flush()
        hmm_read = x.read_model()

        journal: Journal = request.app.state.journal
        x = PressRequest.create(hmm_read.job.id, hmm_read.file, gencode, epsilon)

        await journal.publish("press", x.model_dump_json())
        session.commit()

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
    storage: Storage = request.app.state.storage
    database: Database = request.app.state.database

    with database.create_session() as session:
        x = HMM.get_by_id(session, hmm_id)
        if x is None:
            raise NotFoundInDatabaseError("HMM")
        if storage.has_file(x.name):
            storage.delete(x.name)
        session.delete(x)
        session.commit()
