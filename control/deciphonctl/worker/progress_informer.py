import urllib.parse
from multiprocessing import JoinableQueue

import requests

from deciphonctl.settings import Settings
from deciphonctl.worker.consumer import Consumer
from deciphonctl.worker.schemas import JobUpdate


class ProgressInformer(Consumer):
    def __init__(self, settings: Settings, qin: JoinableQueue):
        super().__init__(qin)
        self._url = settings.sched_url.unicode_string()

    def callback(self, message: str):
        x = JobUpdate.model_validate_json(message)
        url = urllib.parse.urljoin(self._url, f"jobs/{x.id}")
        json = {"state": x.state.value, "progress": x.progress, "error": x.error}
        requests.patch(url, json=json)
