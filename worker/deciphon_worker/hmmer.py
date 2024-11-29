from functools import partial
from typing import Any

from h3daemon.hmmfile import HMMFile
from h3daemon.sched import SchedContext
from loguru import logger

from deciphon_worker.thread import launch_thread

info = logger.info


class HMMER:
    def __init__(self, hmmfile: HMMFile, stdout: Any, stderr: Any):
        info("starting hmmer daemon")
        h3file = HMMFile(hmmfile.path)
        stdout = stdout
        stderr = stderr
        self._sched_ctx = SchedContext(h3file, stdout=stdout, stderr=stderr)
        self._sched = self._sched_ctx.__enter__()

    def shutdown(self):
        info("stopping hmmer daemon")
        if self._sched_ctx is not None:
            self._sched_ctx.__exit__()
            self._sched_ctx = None

    @property
    def port(self):
        return self._sched.get_cport()


def launch_hmmer(hmmfile: HMMFile, stdout: Any = None, stderr: Any = None):
    return launch_thread(partial(HMMER, hmmfile, stdout, stderr), name="HMMER")
