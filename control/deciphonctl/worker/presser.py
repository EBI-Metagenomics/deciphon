import shutil
from multiprocessing import JoinableQueue
from pathlib import Path

from deciphon_core.press import PressContext
from deciphon_core.schema import HMMFile
from loguru import logger
from pydantic import FilePath, HttpUrl

from deciphonctl.download import download
from deciphonctl.presigned import Presigned
from deciphonctl.settings import Settings
from deciphonctl.tempfiles import Tempfiles
from deciphonctl.worker.consumer import Consumer
from deciphonctl.worker.permissions import normalise_file_permissions
from deciphonctl.worker.progress_logger import ProgressLogger
from deciphonctl.worker.schemas import JobState, JobUpdate, PressRequest
from deciphonctl.worker.upload import upload
from deciphonctl.worker.url import url_filename


def run_update(job_id: int, progress: int):
    return JobUpdate(
        id=job_id,
        state=JobState.run,
        progress=progress,
        error="",
    )


def done_update(job_id: int):
    return JobUpdate(
        id=job_id,
        state=JobState.done,
        progress=100,
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
    def __init__(self, settings: Settings, qin: JoinableQueue, qout: JoinableQueue):
        super().__init__(qin)
        self._presigned = Presigned(settings.sched_url)
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
                self._qout.put(done_update(request.job_id).model_dump_json())
        normalise_file_permissions(dcpfile)
        return dcpfile

    def callback(self, message: str):
        x = PressRequest.model_validate_json(message)

        try:
            logger.info(f"downloading {x.hmm.name}")
            url = self._presigned.download_hmm_url(x.hmm.name)
            hmmfile = Path(url_filename(url))

            dcpfile = self._hmm2dcp(url, hmmfile, x)
            logger.info(f"finished creating {dcpfile}")

            url, fields = self._presigned.upload_db_post(dcpfile.name)
            upload(dcpfile, url, fields)
            logger.info(f"finished uploading {dcpfile}")
        except Exception as exception:
            self._qout.put(fail_update(x.job_id, str(exception)).model_dump_json())
            raise exception
