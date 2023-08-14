import shutil
from multiprocessing import JoinableQueue, Process
from pathlib import Path

from deciphon_core.press import PressContext
from deciphon_core.schema import HMMFile
from loguru import logger
from pydantic import FilePath, HttpUrl

from deciphonctl.consumer import Consumer
from deciphonctl.download import download
from deciphonctl.models import DBFile, JobState, JobUpdate, PressRequest
from deciphonctl.permissions import normalise_file_permissions
from deciphonctl.progress_informer import ProgressInformer
from deciphonctl.progress_logger import ProgressLogger
from deciphonctl.sched import Sched
from deciphonctl.tempfiles import Tempfiles
from deciphonctl.url import url_filename
from deciphonctl.worker import worker_loop


def run_update(job_id: int, progress: int):
    return JobUpdate(
        id=job_id,
        state=JobState.run,
        progress=progress,
        error="",
    )


def fail_update(job_id: int, error: str):
    return JobUpdate(
        id=job_id,
        state=JobState.fail,
        progress=0,
        error=error,
    )


class Presser(Consumer):
    def __init__(self, sched: Sched, qin: JoinableQueue, qout: JoinableQueue):
        super().__init__(qin)
        self._sched = sched
        self._tempfiles = Tempfiles()
        self._tempfiles.cleanup()
        self._qout = qout

    def _hmm2dcp(self, url: HttpUrl, hmmfile: Path, request: PressRequest):
        dcpfile = hmmfile.with_suffix(".dcp")
        hmmtmp = self._tempfiles.unique(hmmfile)
        try:
            logger.info(f"downloading {url}")
            download(url, hmmtmp)
            logger.info(f"pressing {hmmtmp}")
            dcptmp = self._press(hmmtmp, request)
            shutil.move(dcptmp, dcpfile)
        finally:
            hmmtmp.unlink(missing_ok=True)
        return dcpfile

    def _press(self, hmmfile: Path, request: PressRequest):
        dcpfile = hmmfile.with_suffix(".dcp")
        hmm = HMMFile(path=FilePath(hmmfile))
        db = request.db
        with PressContext(hmm, gencode=db.gencode, epsilon=db.epsilon) as press:
            with ProgressLogger(press.nproteins, str(hmmfile)) as progress:
                for x in [press] * press.nproteins:
                    x.next()
                    progress.consume()
                    perc = int(round(progress.percent))
                    self._qout.put(run_update(request.job_id, perc).model_dump_json())
        normalise_file_permissions(dcpfile)
        return dcpfile

    def callback(self, message: str):
        x = PressRequest.model_validate_json(message)

        try:
            logger.info(f"downloading {x.hmm.name}")
            url = self._sched.presigned.download_hmm_url(x.hmm.name)
            hmmfile = Path(url_filename(url))

            dcpfile = self._hmm2dcp(url, hmmfile, x)
            logger.info(f"finished creating {dcpfile}")

            self._sched.upload(
                dcpfile, self._sched.presigned.upload_db_post(dcpfile.name)
            )
            logger.info(f"finished uploading {dcpfile}")

            self._sched.db_post(
                DBFile(name=dcpfile.name, gencode=x.db.gencode, epsilon=x.db.epsilon)
            )
            logger.info(f"finished posting {dcpfile}")

        except Exception as exception:
            self._qout.put(fail_update(x.job_id, str(exception)).model_dump_json())
            raise exception


def presser_entry(sched: Sched, num_workers: int):
    qin = JoinableQueue()
    qout = JoinableQueue()
    informer = ProgressInformer(sched, qout)
    pressers = [Presser(sched, qin, qout) for _ in range(num_workers)]
    consumers = [Process(target=x.entry_point, daemon=True) for x in pressers]
    consumers += [Process(target=informer.entry_point, daemon=True)]
    worker_loop(qin, consumers)
