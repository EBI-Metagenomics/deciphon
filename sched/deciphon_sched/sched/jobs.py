from fastapi import APIRouter, Request
from starlette.status import HTTP_200_OK

from deciphon_sched.database import Database
from deciphon_sched.errors import NotFoundInDatabaseError
from deciphon_sched.sched.models import Job
from deciphon_sched.sched.schemas import JobRead, JobState, JobUpdate


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


@router.patch("/jobs/{job_id}", status_code=HTTP_200_OK)
async def update_job_state(request: Request, job_id: int, job: JobUpdate) -> JobRead:
    database: Database = request.app.state.database
    with database.create_session() as session:
        x = Job.get_by_id(session, job_id)
        if x is None:
            raise NotFoundInDatabaseError("Job")

        if job.state == JobState.pend:
            x.set_pend()
        if job.state == JobState.run:
            x.set_run(job.progress)
        elif job.state == JobState.done:
            x.set_done()
        elif job.state == JobState.fail:
            x.set_fail(job.error)
        else:
            assert False

        session.commit()
        return x.read_model()
