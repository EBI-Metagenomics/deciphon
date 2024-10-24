from functools import partial
from queue import Queue
from subprocess import DEVNULL
from threading import Thread

import psutil
from deciphon_core.batch import Batch
from deciphon_core.scan import Scan
from deciphon_core.schema import HMMFile, NewSnapFile
from deciphon_core.sequence import Sequence
from deciphon_poster.poster import Poster
from deciphon_poster.schema import JobUpdate
from loguru import logger

from deciphon_worker.alarm import Alarm
from deciphon_worker.background import Background
from deciphon_worker.files import unique_temporary_file
from deciphon_worker.h3daemon import H3Daemon
from deciphon_worker.models import ScanRequest


def progress_checker(job_id: int, bg: Background, poster: Poster, scan: Scan):
    percent = scan.progress()
    bg.fire(partial(poster.job_patch, JobUpdate.run(job_id, percent)))


class ScanThread:
    def __init__(
        self,
        bg: Background,
        poster: Poster,
        hmmfile: HMMFile,
        multi_hits: bool,
        hmmer3_compat: bool,
    ):
        self._background = bg
        self._poster = poster
        self._hmmfile = hmmfile
        self._multi_hits = multi_hits
        self._hmmer3_compat = hmmer3_compat
        self._daemon = H3Daemon(self._hmmfile, stdout=DEVNULL)
        self._scan: Scan | None = None
        self._queue: Queue[ScanRequest | None] = Queue()
        self._thread = Thread(target=self.run)

    def start(self):
        self._thread.start()

    def stop(self):
        self._queue.put(None)
        self._thread.join()

    def fire(self, request: ScanRequest):
        self._queue.put(request)

    def __enter__(self):
        logger.info("starting h3daemon")
        self._daemon.start()

        logger.info("starting scanner")
        self._scan = Scan(
            self._hmmfile.dbfile,
            self._daemon.port,
            psutil.cpu_count(),
            self._multi_hits,
            self._hmmer3_compat,
            True,
        )
        return self

    def __exit__(self, *_):
        if self._scan is not None:
            self._scan.free()
        self._daemon.stop()

    def run(self):
        with self:
            while True:
                request = self._queue.get()
                try:
                    if request is None:
                        break
                    self.process(request)
                finally:
                    self._queue.task_done()

    def process(self, request: ScanRequest):
        batch = Batch()
        for seq in request.seqs:
            batch.add(Sequence(seq.id, seq.name, seq.data))

        with unique_temporary_file(".dcs") as t:
            snap = NewSnapFile(path=t)
            assert self._scan is not None

            callback = partial(
                progress_checker,
                request.job_id,
                self._background,
                self._poster,
                self._scan,
            )
            with Alarm(1, callback):
                self._scan.run(snap, batch)

        if self._scan.interrupted:
            raise InterruptedError("Scanner has been interrupted.")

        snap.make_archive()
        logger.info(f"Scan has finished successfully and results in '{snap.path}'.")
        self._poster.snap_post(request.id, snap.path)
        snap.path.unlink(True)
