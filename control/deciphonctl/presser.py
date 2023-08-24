import shutil
from multiprocessing import JoinableQueue, Process
from pathlib import Path

from deciphon_core.press import PressContext
from deciphon_core.schema import HMMFile
from loguru import logger
from pydantic import FilePath, HttpUrl

from deciphonctl.consumer import Consumer
from deciphonctl.download import download
from deciphonctl.files import atomic_file_creation, remove_temporary_files
from deciphonctl.models import DBFile, JobUpdate, PressRequest
from deciphonctl.permissions import normalise_file_permissions
from deciphonctl.progress_informer import ProgressInformer
from deciphonctl.progress_logger import ProgressLogger
from deciphonctl.sched import Sched
from deciphonctl.settings import Settings
from deciphonctl.url import url_filename
from deciphonctl.worker import worker_loop


class Presser(Consumer):
    def __init__(self, sched: Sched, qin: JoinableQueue, qout: JoinableQueue):
        super().__init__(qin)
        self._sched = sched
        remove_temporary_files()
        self._qout = qout

    def _hmm2dcp(self, url: HttpUrl, hmmfile: Path, request: PressRequest):
        dcpfile = hmmfile.with_suffix(".dcp")
        with atomic_file_creation(hmmfile) as x:
            logger.info(f"downloading {url}")
            download(url, x)
            logger.info(f"pressing {x}")
            dcptmp = self._press(x, request)
            shutil.move(dcptmp, dcpfile)
        return dcpfile

    def _press(self, hmmfile: Path, req: PressRequest):
        dcpfile = hmmfile.with_suffix(".dcp")
        hmm = HMMFile(path=FilePath(hmmfile))
        db = req.db
        with PressContext(hmm, gencode=db.gencode, epsilon=db.epsilon) as press:
            self._qout.put(JobUpdate.run(req.job_id, 0).model_dump_json())
            with ProgressLogger(press.nproteins, str(hmmfile)) as progress:
                for x in [press] * press.nproteins:
                    x.next()
                    progress.consume()
                    perc = int(round(progress.percent))
                    self._qout.put(JobUpdate.run(req.job_id, perc).model_dump_json())
        normalise_file_permissions(dcpfile)
        return dcpfile

    def callback(self, message: str):
        x = PressRequest.model_validate_json(message)

        try:
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
            self._qout.put(JobUpdate.fail(x.job_id, str(exception)).model_dump_json())
            raise exception


def presser_entry(settings: Settings, sched: Sched, num_workers: int):
    qin = JoinableQueue()
    qout = JoinableQueue()
    informer = ProgressInformer(sched, qout)
    pressers = [Presser(sched, qin, qout) for _ in range(num_workers)]
    consumers = [Process(target=x.run, daemon=True) for x in pressers]
    consumers += [Process(target=informer.run, daemon=True)]
    worker_loop(settings, f"/{settings.mqtt_topic}/press", qin, consumers)
