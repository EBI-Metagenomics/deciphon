from typing import List

from fastapi import APIRouter, Depends
from starlette.status import HTTP_200_OK, HTTP_201_CREATED, HTTP_204_NO_CONTENT

from deciphon_api.auth import auth_request
from deciphon_api.errors import FileNotInStorageError
from deciphon_api.journal import get_journal
from deciphon_api.models import HMM, HMMCreate, HMMRead, Job, JobType
from deciphon_api.sched import get_sched, select
from deciphon_api.storage import storage_has

__all__ = ["router"]

router = APIRouter()

OK = HTTP_200_OK
NO_CONTENT = HTTP_204_NO_CONTENT
CREATED = HTTP_201_CREATED
AUTH = [Depends(auth_request)]


@router.get("/hmms", response_model=List[HMMRead], status_code=OK)
async def read_hmms():
    with get_sched() as sched:
        return [HMMRead.from_orm(x) for x in sched.exec(select(HMM)).all()]


@router.post("/hmms/", response_model=HMMRead, status_code=CREATED, dependencies=AUTH)
async def create_hmm(hmm: HMMCreate):
    if not storage_has(hmm.sha256):
        raise FileNotInStorageError(hmm.sha256)

    with get_sched() as sched:
        x = HMM.from_orm(hmm)
        x.job = Job(type=JobType.hmm)
        sched.add(x)
        sched.commit()
        sched.refresh(x)
        y = HMMRead.from_orm(x)
        await get_journal().publish_hmm(y.id)
        return y


@router.get("/hmms/{hmm_id}", response_model=HMMRead, status_code=OK)
async def read_hmm(hmm_id: int):
    with get_sched() as sched:
        return HMMRead.from_orm(sched.get(HMM, hmm_id))


@router.delete("/hmms/{hmm_id}", status_code=NO_CONTENT, dependencies=AUTH)
async def delete_hmm(hmm_id: int):
    with get_sched() as sched:
        sched.delete(sched.get(HMM, hmm_id))
        sched.commit()
