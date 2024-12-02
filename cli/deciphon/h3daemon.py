from deciphon_schema import HMMFile
from h3daemon.sched import SchedContext

__all__ = ["H3Daemon"]


class H3Daemon:
    def __init__(self, hmmfile: HMMFile, stdout=None, stderr=None) -> None:
        self._sched_ctx = SchedContext(hmmfile, stdout=stdout, stderr=stderr)
        self._port: int = -1

    @property
    def port(self):
        return self._port

    def __enter__(self):
        sched = self._sched_ctx.__enter__()
        self._port = sched.get_cport()
        return self

    def __exit__(self, *_):
        self._sched_ctx.__exit__(*_)
