from fastapi import APIRouter, Request
from starlette.status import HTTP_200_OK

from deciphon_sched.database import Database
from deciphon_sched.journal import Journal
from deciphon_sched.sched.schemas import HealthCheck
from deciphon_sched.storage import Storage

router = APIRouter()


@router.get("/health", status_code=HTTP_200_OK)
async def read_health(request: Request) -> HealthCheck:
    database: Database = request.app.state.database
    database.health_check()

    journal: Journal = request.app.state.journal
    await journal.health_check()

    storage: Storage = request.app.state.storage
    storage.health_check()
    return HealthCheck(status="OK")
