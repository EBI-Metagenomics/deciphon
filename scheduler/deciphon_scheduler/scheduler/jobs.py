from fastapi import APIRouter, Request
from starlette.status import HTTP_200_OK

from deciphon_scheduler.database import Database
from deciphon_scheduler.errors import NotFoundInDatabaseError
from deciphon_scheduler.scheduler.models import Job
from deciphon_scheduler.scheduler.schemas import JobRead, JobUpdate


router = APIRouter()


@router.get("/jobs", status_code=HTTP_200_OK)
async def read_jobs(request: Request) -> list[JobRead]:
    database: Database = request.app.state.database
    with database.create_session() as session:
        return [x.read_model() for x in Job.get_all(session)]


@router.get("/jobs/{job_id}", status_code=HTTP_200_OK)
async def read_job(request: Request, job_id: int) -> JobRead:
    database: Database = request.app.state.database
    with database.create_session() as session:
        x = Job.get_by_id(session, job_id)
        if x is None:
            raise NotFoundInDatabaseError("Job")
        return x.read_model()


@router.patch("/jobs/", status_code=HTTP_200_OK)
async def update_job(request: Request, job: JobUpdate) -> JobRead:
    database: Database = request.app.state.database
    with database.create_session() as session:
        x = Job.get_by_id(session, job.id)
        if x is None:
            raise NotFoundInDatabaseError("Job")

        for key in job.model_fields.keys():
            if getattr(job, key) is not None:
                setattr(x, key, getattr(job, key))

        session.commit()
        return x.read_model()