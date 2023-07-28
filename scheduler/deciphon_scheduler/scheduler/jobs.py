from fastapi import APIRouter, Request
from starlette.status import HTTP_200_OK, HTTP_204_NO_CONTENT

from ..database import Database
from ..errors import NotFoundInDatabaseError
from .models import Job
from .schemas import JobRead, JobUpdate

__all__ = ["router"]

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


@router.delete("/jobs/{job_id}", status_code=HTTP_204_NO_CONTENT)
async def delete_job(request: Request, job_id: int):
    database: Database = request.app.state.database
    with database.create_session() as session:
        x = Job.get_by_id(session, job_id)
        if x is None:
            raise NotFoundInDatabaseError("Job")
        session.delete(x)
        session.commit()


#
#
# @router.get("/jobs/{job_id}/scan", response_model=ScanRead, status_code=OK)
# async def read_job_scan(job_id: int):
#     with get_sched() as sched:
#         job = sched.get(Job, job_id)
#         return ScanRead.from_orm(job.scan)
