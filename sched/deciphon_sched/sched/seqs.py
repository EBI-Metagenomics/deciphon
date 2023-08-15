from fastapi import APIRouter, Request
from starlette.status import HTTP_200_OK

from deciphon_sched.database import Database
from deciphon_sched.errors import NotFoundInDatabaseError
from deciphon_sched.sched.models import Seq
from deciphon_sched.sched.schemas import SeqRead

router = APIRouter()


@router.get("/seqs", status_code=HTTP_200_OK)
async def read_seqs(request: Request) -> list[SeqRead]:
    database: Database = request.app.state.database
    with database.create_session() as session:
        return [x.read_model() for x in Seq.get_all(session)]


@router.get("/seqs/{seq_id}", status_code=HTTP_200_OK)
async def read_seq(request: Request, seq_id: int) -> SeqRead:
    database: Database = request.app.state.database
    with database.create_session() as session:
        x = Seq.get_by_id(session, seq_id)
        if x is None:
            raise NotFoundInDatabaseError("Seq")
        return x.read_model()
