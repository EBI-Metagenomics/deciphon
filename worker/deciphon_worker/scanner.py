import shutil
import sys
from dataclasses import dataclass
from functools import partial
from threading import Thread
from typing import Any

from deciphon_core.scan import Scan
from deciphon_core.sequence import Sequence
from deciphon_schema import DBFile, HMMFile, NewSnapFile, SnapFile
from loguru import logger

from deciphon_worker.alarm import Alarm
from deciphon_worker.batch import create_batch
from deciphon_worker.hmmer import HMMER, launch_hmmer
from deciphon_worker.interrupted import Interrupted
from deciphon_worker.progressor import Progressor
from deciphon_worker.queue_loop import queue_loop
from deciphon_worker.thread import launch_thread

if sys.version_info >= (3, 13):
    from queue import Queue
else:
    from deciphon_worker.queue313 import Queue


info = logger.info


@dataclass
class Request:
    snap: NewSnapFile
    sequences: list[Sequence]
    future: Progressor[SnapFile]


class Scanner:
    def __init__(
        self,
        dbfile: DBFile,
        num_threads: int = 2,
        multi_hits: bool = True,
        hmmer3_compat: bool = False,
        cache: bool = False,
        stdout: Any = None,
        stderr: Any = None,
    ):
        hmmfile = HMMFile(path=dbfile.hmmpath.path)
        self._hmmer: HMMER = launch_hmmer(hmmfile, stdout, stderr).result()
        info("starting scan daemon")
        self._scan = Scan(
            dbfile,
            self._hmmer.port,
            num_threads,
            multi_hits,
            hmmer3_compat,
            cache,
        )
        self._queue: Queue[Request] = Queue()
        self._is_stop = False
        self._thread = Thread(target=self._run)
        self._thread.start()

    def shutdown(self):
        def func():
            info("stopping scan daemon")
            self._is_stop = True
            self._queue.shutdown(immediate=True)
            self._thread.join()
            self._scan.free()
            self._hmmer.shutdown()

        return launch_thread(func, name="Scanner.shutdown")

    def put(self, snap: NewSnapFile, sequences: list[Sequence]):
        future: Progressor[SnapFile] = Progressor()
        request = Request(snap, sequences, future)
        self._queue.put(request)
        return future

    def _run(self):
        for x in queue_loop(self._queue):
            x.future.set_running_or_notify_cancel()
            try:
                batch = create_batch(x.sequences)
                scan = self._scan

                with Alarm(0.1, partial(self._monitor, x.future)):
                    scan.run(x.snap, batch)

                if scan.interrupted:
                    info("Scanner has been interrupted.")
                    raise Interrupted

                x.snap.make_archive()
                snap = SnapFile(path=x.snap.path)
                info(f"Scan has finished successfully and results in '{snap.path}'.")
                x.future.set_progress(100)
                x.future.set_result(snap)
            except Exception as exception:
                shutil.rmtree(x.snap.path, ignore_errors=True)
                x.future.set_exception(exception)
            finally:
                shutil.rmtree(x.snap.basedir, ignore_errors=True)

    def _monitor(self, x: Progressor[SnapFile]):
        scan = self._scan
        if x.interrupted or self._is_stop:
            scan.interrupt()
            return
        x.set_progress(scan.progress())


def launch_scanner(
    dbfile: DBFile,
    num_threads: int = 2,
    multi_hits: bool = True,
    hmmer3_compat: bool = False,
    cache: bool = False,
    stdout: Any = None,
    stderr: Any = None,
):
    func = partial(
        Scanner,
        dbfile=dbfile,
        num_threads=num_threads,
        multi_hits=multi_hits,
        hmmer3_compat=hmmer3_compat,
        cache=cache,
        stdout=stdout,
        stderr=stderr,
    )
    return launch_thread(func, name="Scanner")
