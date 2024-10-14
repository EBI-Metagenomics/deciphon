from multiprocessing import JoinableQueue, Process
from pathlib import Path
from subprocess import DEVNULL

from deciphon.h3daemon import H3Daemon
from deciphon_core.scan import DeciphonError, Scan
from deciphon_core.batch import Batch
from deciphon_core.schema import DBFile, HMMFile, NewSnapFile
from deciphon_core.sequence import Sequence
from loguru import logger

from deciphonctl.consumer import Consumer
from deciphonctl.download import download
from deciphonctl.file_path import file_path
from deciphonctl.files import (
    atomic_file_creation,
    remove_temporary_files,
    unique_temporary_file,
)
from deciphonctl.models import JobUpdate, ScanRequest
from deciphonctl.progress import Progress
from deciphonctl.progress_informer import ProgressInformer
from deciphonctl.sched import Sched
from deciphonctl.settings import Settings
from deciphonctl.worker import worker_loop


class Scanner(Consumer):
    def __init__(
        self,
        sched: Sched,
        qin: JoinableQueue,
        qout: JoinableQueue,
        num_threads: int,
        cache: bool,
    ):
        super().__init__(qin)
        self._num_threads = num_threads
        self._cache = cache
        self._sched = sched
        self._daemon: H3Daemon | None = None
        self._scan: Scan | None = None
        self._hmmfile = None
        self._multi_hits = None
        self._hmmer3_compat = None
        remove_temporary_files()
        self._qout = qout
        self._batch: Batch | None = None

    def callback(self, message: str):
        x = ScanRequest.model_validate_json(message)

        hmmfile = Path(x.hmm.name)
        dbfile = Path(x.db.name)

        if not hmmfile.exists():
            with atomic_file_creation(hmmfile) as t:
                download(self._sched.presigned.download_hmm_url(hmmfile.name), t)

        if not dbfile.exists():
            with atomic_file_creation(dbfile) as t:
                download(self._sched.presigned.download_db_url(dbfile.name), t)
        if self._batch is None:
            self._batch = Batch()

        with unique_temporary_file(".dcs") as t:
            snap = NewSnapFile(path=t)

            db = DBFile(path=file_path(dbfile))

            if (
                self._daemon is None
                or self._scan is None
                or self._hmmfile != hmmfile
                or self._multi_hits != x.multi_hits
                or self._hmmer3_compat != x.hmmer3_compat
            ):
                self._hmmfile = hmmfile
                self._multi_hits = x.multi_hits
                self._hmmer3_compat = x.hmmer3_compat

                if self._scan is not None:
                    self._scan.__exit__()
                if self._daemon is not None:
                    self._daemon.__exit__()

                self._scan = None
                self._daemon = None

                logger.info("starting h3daemon")
                self._daemon = H3Daemon(
                    HMMFile(path=file_path(hmmfile)), stdout=DEVNULL
                )
                logger.info("self._daemon.__enter__()")
                self._daemon.__enter__()
                logger.info("starting scanner")
                self._scan = Scan(
                    db,
                    self._daemon.port,
                    self._num_threads,
                    x.multi_hits,
                    x.hmmer3_compat,
                    self._cache,
                )
                self._scan.__enter__()

            self._batch.reset()
            try:
                with Progress("scan", self._scan, self._sched, x.job_id):
                    for seq in x.seqs:
                        self._batch.add(Sequence(seq.id, seq.name, seq.data))
                    self._scan.run(snap, self._batch)
            except DeciphonError as ex:
                logger.exception(ex)
                self._sched.job_patch(JobUpdate.fail(x.job_id, str(ex)))
                return
            if self._scan.interrupted:
                ex = InterruptedError("Scanner has been interrupted.")
                self._sched.job_patch(JobUpdate.fail(x.job_id, str(ex)))
                raise ex
            snap.make_archive()
            logger.info(
                "Scan has finished successfully and "
                f"results stored in '{snap.path}'."
            )
            self._sched.snap_post(x.id, snap.path)


def scanner_entry(
    settings: Settings, sched: Sched, num_workers: int, num_threads: int, cache: bool
):
    qin = JoinableQueue()
    qout = JoinableQueue()
    informer = ProgressInformer(sched, qout)
    pressers = [
        Scanner(sched, qin, qout, num_threads, cache) for _ in range(num_workers)
    ]
    consumers = [Process(target=x.run, daemon=True) for x in pressers]
    consumers += [Process(target=informer.run, daemon=True)]
    worker_loop(settings, f"/{settings.mqtt_topic}/scan", qin, consumers)
