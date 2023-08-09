import shutil
from pathlib import Path
from typing import Any

from deciphon_core.hmmfile import HMMFile
from deciphon_core.press import PressContext
from pydantic import FilePath, Json

from deciphon_worker.download import download
from deciphon_worker.models import HMMRequest
from deciphon_worker.permissions import normalise_file_permissions
from deciphon_worker.presigned import Presigned
from deciphon_worker.producer import Producer
from deciphon_worker.settings import Settings
from deciphon_worker.tempfiles import Tempfiles
from deciphon_worker.url_filename import url_filename


def press(hmmfile: Path, gencode: int, epsilon: float):
    dcpfile = hmmfile.with_suffix(".dcp")
    hmm = HMMFile(path=FilePath(hmmfile))
    with PressContext(hmm, gencode=gencode, epsilon=epsilon) as press:
        for x in [press] * press.nproteins:
            x.next()
    normalise_file_permissions(dcpfile)
    return dcpfile


class Presser(Producer):
    def __init__(self, settings: Settings, num_workers: int):
        topic = f"/{settings.mqtt_topic}/hmms"
        super().__init__(settings.mqtt_host, settings.mqtt_port, topic, num_workers)
        self._presigned = Presigned(settings)
        self._tempfiles = Tempfiles()

    def consume(self, message: Json[Any]):
        hmm = HMMRequest.model_validate_json(message)
        try:
            url = self._presigned.download_hmm_url(hmm.name)
            hmmfile = Path(url_filename(url))
        except Exception as exception:
            print(exception)
            return

        dcpfile = hmmfile.with_suffix(".dcp")
        hmmtmp = self._tempfiles.unique(hmmfile)

        try:
            download(url, hmmtmp)
            dcptmp = press(hmmtmp, 1, 0.01)
            shutil.move(dcptmp, dcpfile)
        except Exception as exception:
            print(exception)
            return
        finally:
            hmmtmp.unlink(missing_ok=True)
