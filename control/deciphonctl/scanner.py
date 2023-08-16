from multiprocessing import JoinableQueue, Process
from pathlib import Path
from subprocess import DEVNULL

from deciphon.h3daemon import H3Daemon
from deciphon_core.scan import Scan
from deciphon_core.scan_params import ScanParams
from deciphon_core.schema import DBFile, HMMFile, NewSnapFile
from deciphon_core.seq import Seq
from pydantic import FilePath

from deciphonctl import settings
from deciphonctl.consumer import Consumer
from deciphonctl.download import download
from deciphonctl.files import (
    atomic_file_creation,
    remove_temporary_files,
    unique_temporary_file,
)
from deciphonctl.models import ScanRequest, JobUpdate
from deciphonctl.progress_informer import ProgressInformer
from deciphonctl.sched import Sched
from deciphonctl.worker import worker_loop
from deciphonctl.progress_logger import ProgressLogger


def sequence_iterator(seqs: list[Seq], job_id: int, desc: str, qout: JoinableQueue):
    qout.put(JobUpdate.run(job_id, 0).model_dump_json())
    with ProgressLogger(len(seqs), desc) as progress:
        for seq in seqs:
            yield seq
            progress.consume()
            perc = int(round(progress.percent))
            qout.put(JobUpdate.run(job_id, perc).model_dump_json())


class Scanner(Consumer):
    def __init__(self, sched: Sched, qin: JoinableQueue, qout: JoinableQueue):
        super().__init__(qin)
        self._sched = sched
        remove_temporary_files()
        self._qout = qout

    def callback(self, message: str):
        x = ScanRequest.model_validate_json(message)
        print(x)

        hmmfile = Path(x.hmm.name)
        dbfile = Path(x.db.name)

        with atomic_file_creation(hmmfile) as t:
            download(self._sched.presigned.download_hmm_url(hmmfile.name), t)

        with atomic_file_creation(dbfile) as t:
            download(self._sched.presigned.download_db_url(dbfile.name), t)

        with unique_temporary_file(".dcs") as t:
            snap = NewSnapFile(path=t)
            print(snap)

            num_threads = 1
            lrt_threshold = 0.0

            db = DBFile(path=FilePath(dbfile))
            print(db)

            seqs = [Seq(seq.id, seq.name, seq.data) for seq in x.seqs]
            seqit = sequence_iterator(seqs, x.job_id, "scan", self._qout)

            with H3Daemon(HMMFile(path=FilePath(hmmfile)), stdout=DEVNULL) as daemon:
                params = ScanParams(
                    num_threads=num_threads,
                    lrt_threshold=lrt_threshold,
                    multi_hits=x.multi_hits,
                    hmmer3_compat=x.hmmer3_compat,
                )
                print(params)
                scan = Scan()
                scan.dial(daemon.port)
                scan.setup(params)
                scan.run(db, seqit, snap)
                print(
                    "Scan has finished successfully and "
                    f"results stored in '{snap.path}'."
                )


def scanner_entry(sched: Sched, num_workers: int):
    qin = JoinableQueue()
    qout = JoinableQueue()
    informer = ProgressInformer(sched, qout)
    pressers = [Scanner(sched, qin, qout) for _ in range(num_workers)]
    consumers = [Process(target=x.entry_point, daemon=True) for x in pressers]
    consumers += [Process(target=informer.entry_point, daemon=True)]
    worker_loop(f"/{settings.mqtt_topic}/scan", qin, consumers)
