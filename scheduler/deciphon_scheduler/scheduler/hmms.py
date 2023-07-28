from fastapi import APIRouter, Request
from sqlalchemy import select
from starlette.status import HTTP_200_OK, HTTP_201_CREATED, HTTP_204_NO_CONTENT

from ..database import Database
from ..errors import FileNameExistsError, NotFoundInDatabaseError
from ..journal import Journal
from .models import HMM
from .schemas import HMMCreate, HMMFile, HMMRead

router = APIRouter()

NO_CONTENT = HTTP_204_NO_CONTENT


@router.get("/hmms", response_model=list[HMMRead], status_code=HTTP_200_OK)
async def read_hmms(request: Request):
    database: Database = request.app.state.database
    with database.create_session() as session:
        return [x[0].read_model() for x in session.execute(select(HMM)).all()]


# , response_model=HMMRead
@router.post("/hmms/", status_code=HTTP_201_CREATED)
async def create_hmm(request: Request, hmm: HMMCreate):
    database: Database = request.app.state.database
    with database.create_session() as session:
        r = session.execute(select(HMM).where(HMM.file_name == hmm.file.name))
        if r.one_or_none() is not None:
            return FileNameExistsError(hmm.file.name)

        x = HMM.create(hmm.file)
        session.add(x)
        session.commit()
        hmm_read = HMMRead(id=x.id, job_id=x.job_id, file=hmm.file)

    journal: Journal = request.app.state.journal
    await journal.publish("hmms", hmm_read.model_dump_json())

    return hmm_read


# response_model=HMMRead,
@router.get("/hmms/{hmm_id}", status_code=HTTP_200_OK)
async def read_hmm(request: Request, hmm_id: int):
    database: Database = request.app.state.database
    with database.create_session() as session:
        x = session.execute(select(HMM).where(HMM.id == hmm_id)).one_or_none()
        if x is None:
            raise NotFoundInDatabaseError("HMM")
        x = x[0]
        file = HMMFile(name=x.file_name, sha256=x.file_sha256)
        return HMMRead(id=x.id, job_id=x.job_id, file=file)


@router.delete("/hmms/{hmm_id}", status_code=HTTP_204_NO_CONTENT)
async def delete_hmm(request: Request, hmm_id: int):
    database: Database = request.app.state.database
    with database.create_session() as session:
        x = session.execute(select(HMM).where(HMM.id == hmm_id)).one_or_none()
        if x is None:
            raise NotFoundInDatabaseError("HMM")
        session.delete(x[0])
        session.commit()