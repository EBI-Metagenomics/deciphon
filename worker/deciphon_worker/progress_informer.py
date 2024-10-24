from multiprocessing import JoinableQueue

from deciphon_worker.consumer import Consumer
from deciphon_worker.models import JobUpdate
from deciphon_worker.sched import Sched


class ProgressInformer(Consumer):
    def __init__(self, sched: Sched, qin: JoinableQueue):
        super().__init__(qin)
        self._sched = sched

    def callback(self, message: str):
        self._sched.job_patch(JobUpdate.model_validate_json(message))
