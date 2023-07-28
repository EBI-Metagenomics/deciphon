from fastapi import APIRouter, Request
from starlette.status import HTTP_200_OK, HTTP_201_CREATED, HTTP_204_NO_CONTENT

from .schemas import HMMRead

router = APIRouter()

OK = HTTP_200_OK
NO_CONTENT = HTTP_204_NO_CONTENT
CREATED = HTTP_201_CREATED


@router.get("/hmms", response_model=list[HMMRead], status_code=OK)
async def read_hmms(request: Request):
    return []
    # session = database.create_session()
    # with get_sched() as sched:
    #     return [HMMRead.from_orm(x) for x in sched.exec(select(HMM)).all()]


# @router.post("/hmms/", response_model=HMMRead, status_code=CREATED, dependencies=AUTH)
# async def create_hmm(hmm: HMMCreate):
#     if not storage_has(hmm.sha256):
#         raise FileNotInStorageError(hmm.sha256)
#
#     with get_sched() as sched:
#         x = HMM.from_orm(hmm)
#         x.job = Job(type=JobType.hmm)
#         sched.add(x)
#         sched.commit()
#         sched.refresh(x)
#         y = HMMRead.from_orm(x)
#         await get_journal().publish_hmm(y.id)
#         return y
#
#
# @router.get("/hmms/{hmm_id}", response_model=HMMRead, status_code=OK)
# async def read_hmm(hmm_id: int):
#     with get_sched() as sched:
#         return HMMRead.from_orm(sched.get(HMM, hmm_id))
#
#
# @router.delete("/hmms/{hmm_id}", status_code=NO_CONTENT, dependencies=AUTH)
# async def delete_hmm(hmm_id: int):
#     with get_sched() as sched:
#         sched.delete(sched.get(HMM, hmm_id))
#         sched.commit()
