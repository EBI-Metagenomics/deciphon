from fastapi import APIRouter, Request
from starlette.status import HTTP_200_OK, HTTP_201_CREATED, HTTP_204_NO_CONTENT

from ..database import Database
from ..errors import FileNameExistsError, NotFoundInDatabaseError
from ..journal import Journal
from .models import HMM
from .schemas import HMMCreate, HMMRead

router = APIRouter()


@router.get("/hmms", status_code=HTTP_200_OK)
async def read_hmms(request: Request) -> list[HMMRead]:
    database: Database = request.app.state.database
    with database.create_session() as session:
        return [x.read_model() for x in HMM.get_all(session)]


@router.post("/hmms/", status_code=HTTP_201_CREATED)
async def create_hmm(request: Request, hmm: HMMCreate) -> HMMRead:
    database: Database = request.app.state.database
    with database.create_session() as session:
        x = HMM.get_by_file_name(session, hmm.file.name)
        if x is not None:
            return FileNameExistsError(hmm.file.name)

        x = HMM.create(hmm.file)
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
