from threading import Event, Thread
from time import sleep

from deciphon_core.scan import Scan

from deciphonctl.models import JobUpdate
from deciphonctl.progress_logger import ProgressLogger
from deciphonctl.sched import Sched


class Progress:
    def __init__(self, desc: str, scan: Scan, sched: Sched, job_id: int):
        self._logger = ProgressLogger(desc)
        self._continue = Event()
        self._scan = scan
        self._sched = sched
        self._job_id = job_id
        self._thread = Thread(target=self.progress_entry)

    def start(self):
        self._logger.start()
        self._thread.start()

    def progress_entry(self):
        self._logger.percent = last_percent = 0
        JobUpdate.run(self._job_id, last_percent).model_dump_json()
        while not self._continue.is_set():
            percent = self._scan.progress()
            if last_percent != percent:
                self._logger.percent = last_percent = percent
                msg = JobUpdate.run(self._job_id, last_percent).model_dump_json()
                self._sched.job_patch(JobUpdate.model_validate_json(msg))
            sleep(1.05)

    def stop(self):
        self._continue.set()
        self._thread.join()
        self._logger.stop()

    def __enter__(self):
        self.start()
        return self

    def __exit__(self, *_):
        self.stop()
