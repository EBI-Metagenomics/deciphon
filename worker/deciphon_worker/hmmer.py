from functools import partial
from typing import Any

from deciphon_schema import HMMFile
import h3daemon
from loguru import logger

from deciphon_worker.thread import launch_thread

info = logger.info


class HMMER:
    def __init__(self, hmmfile: HMMFile, stdout: Any, stderr: Any):
        pidfile = h3daemon.spawn(
            hmmfile, stdout=stdout, stderr=stderr, detach=False, force=True
        )
        self._manager = h3daemon.possess(pidfile)
        self._manager.wait_for_readiness()

    def shutdown(self, force=False):
        self._manager.shutdown(force=force)

    @property
    def port(self):
        return self._manager.port()

    def __enter__(self):
        return self

    def __exit__(self, *_):
        self.shutdown()
        return False


def launch_hmmer(hmmfile: HMMFile, stdout: Any = None, stderr: Any = None):
    return launch_thread(partial(HMMER, hmmfile, stdout, stderr), name="HMMER")
