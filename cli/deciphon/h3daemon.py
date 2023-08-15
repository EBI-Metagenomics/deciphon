from deciphon_core.schema import HMMFile
from h3daemon.hmmfile import HMMFile as H3File
from h3daemon.sched import SchedContext

__all__ = ["H3Daemon"]


class H3Daemon:
    def __init__(self, hmmfile: HMMFile, stdout=None, stderr=None) -> None:
        self._hmmfile = hmmfile
        h3file = H3File(hmmfile.path)
        self._sched_ctx = SchedContext(h3file, stdout=stdout, stderr=stderr)
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
