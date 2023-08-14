from multiprocessing import JoinableQueue

from deciphonctl.consumer import Consumer
from deciphonctl.models import JobUpdate
from deciphonctl.sched import Sched


class ProgressInformer(Consumer):
    def __init__(self, sched: Sched, qin: JoinableQueue):
        super().__init__(qin)
        self._sched = sched

    def callback(self, message: str):
        self._sched.job_patch(JobUpdate.model_validate_json(message))
