from __future__ import annotations

from functools import partial
from pathlib import Path

from deciphon_core.schema import HMMFile, HMMName
from deciphon_poster.poster import Poster
from deciphon_poster.schema import JobUpdate
from loguru import logger
from paho.mqtt.client import CallbackAPIVersion, Client

from deciphon_worker.background import Background
from deciphon_worker.download import download
from deciphon_worker.files import atomic_file_creation
from deciphon_worker.models import ScanRequest
from deciphon_worker.queue313 import Queue, ShutDown
from deciphon_worker.scan_thread import ScanThread

FILE_MODE = 0o640
TOPIC = "/deciphon.org/scan"


def on_connect(client, userdata, flags, reason_code, properties):
    logger.info(f"connected to MQTT with result code {reason_code}")
    logger.info(f"subscribing to {TOPIC}")
    client.subscribe(TOPIC)


def on_message(client, userdata, msg):
    assert isinstance(msg.payload, bytes)
    payload = msg.payload.decode()
    logger.info(f"received <{payload}>")
    requests: Queue[ScanRequest] = userdata
    requests.put(ScanRequest.model_validate_json(payload))


def scanner_hash(hmm: HMMName, multi_hits: bool, hmmer3_compat: bool):
    return hash(f"{str(hmm)}_{multi_hits}_{hmmer3_compat}")


def process_request(
    scans: dict[int, ScanThread],
    bg: Background,
    poster: Poster,
    request: ScanRequest,
):
    logger.info(f"processing scan request: {request}")
    bg.fire(partial(poster.job_patch, JobUpdate.run(request.job_id, 0)))

    hmmfile = Path(request.hmm.name)
    dbfile = Path(request.db.name)

    if not hmmfile.exists():
        with atomic_file_creation(hmmfile) as t:
            download(poster.download_hmm_url(hmmfile.name), t)

    if not dbfile.exists():
        with atomic_file_creation(dbfile) as t:
            download(poster.download_db_url(dbfile.name), t)

    id = scanner_hash(request.hmm, request.multi_hits, request.hmmer3_compat)
    if id not in scans:
        hmm = HMMFile(path=hmmfile)
        scans[id] = ScanThread(
            bg, poster, hmm, request.multi_hits, request.hmmer3_compat
        )
        scans[id].start()

    scans[id].fire(request)


class ScannerManager:
    def __init__(self, poster: Poster, mqtt_host: str, mqtt_port: int):
        self.poster = poster
        self.requests: Queue[ScanRequest] = Queue()
        self.scans: dict[int, ScanThread] = dict()
        self.mqtt_host = mqtt_host
        self.mqtt_port = mqtt_port

    def run_forever(self):
        logger.info(
            f"connecting to MQTT server (host={self.mqtt_host}, port={self.mqtt_port})"
        )
        client = Client(CallbackAPIVersion.VERSION2, userdata=self.requests)
        client.on_connect = on_connect
        client.on_message = on_message
        client.connect(self.mqtt_host, self.mqtt_port)

        client.loop_start()
        with Background() as bg:
            while True:
                try:
                    request = self.requests.get()
                except ShutDown:
                    logger.info("shutting down...")
                    break
                try:
                    process_request(self.scans, bg, self.poster, request)
                except Exception as exception:
                    logger.warning(f"scanning failed: {exception}")
                    job_update = JobUpdate.fail(request.job_id, str(exception))
                    bg.fire(partial(self.poster.job_patch, job_update))
                finally:
                    self.requests.task_done()
        client.loop_stop()

        for x in self.scans.values():
            x.stop()

    def stop(self):
        self.requests.shutdown()
