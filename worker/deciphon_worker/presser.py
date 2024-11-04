import os
from pathlib import Path
from deciphon_worker.queue313 import Queue, ShutDown

from deciphon_core.press import PressContext
from deciphon_core.schema import DBName, Gencode, HMMFile
from deciphon_poster.poster import Poster
from deciphon_poster.schema import JobUpdate
from loguru import logger
from paho.mqtt.client import CallbackAPIVersion, Client
from functools import partial

from deciphon_worker.background import Background
from deciphon_worker.download import download
from deciphon_worker.models import PressRequest
from deciphon_worker.progress_logger import ProgressLogger
from deciphon_worker.url import url_filename

FILE_MODE = 0o640
TOPIC = "/deciphon.org/press"


def on_connect(client, userdata, flags, reason_code, properties):
    logger.info(f"connected to MQTT with result code {reason_code}")
    logger.info(f"subscribing to {TOPIC}")
    client.subscribe(TOPIC)


def on_message(client, userdata, msg):
    assert isinstance(msg.payload, bytes)
    payload = msg.payload.decode()
    logger.info(f"received <{payload}>")
    requests: Queue[PressRequest] = userdata
    requests.put(PressRequest.model_validate_json(payload))


def press(
    bg: Background,
    poster: Poster,
    hmm: HMMFile,
    job_id: int,
    gencode: Gencode,
    epsilon: float,
):
    with PressContext(hmm, gencode=gencode, epsilon=epsilon) as press:
        last_percent = 0
        bg.fire(lambda: poster.job_patch(JobUpdate.run(job_id, 0)))
        with ProgressLogger(str(hmm)) as progress:
            for i in range(press.nproteins):
                press.next()
                percent = progress.percent = (100 * (i + 1)) // press.nproteins
                if percent != last_percent:
                    bg.fire(partial(poster.job_patch, JobUpdate.run(job_id, percent)))
                    last_percent = percent


def process_request(bg: Background, poster: Poster, request: PressRequest):
    logger.info(f"processing press request: {request}")

    url = poster.download_hmm_url(request.hmm.name)
    hmmfile = Path(url_filename(url))
    hmmfile.touch(mode=FILE_MODE)
    hmm = HMMFile(path=hmmfile)
    hmm.newdbfile.path.unlink(True)

    logger.info(f"downloading {url}")
    download(url, hmm.path)
    os.chmod(hmm.path, FILE_MODE)

    logger.info(f"pressing {hmm.path}")
    press(bg, poster, hmm, request.job_id, request.gencode, request.epsilon)

    os.chmod(hmm.dbfile.path, FILE_MODE)

    logger.info(f"finished generating database: {hmm.dbfile}")

    logger.info(f"uploading {hmm.dbfile}")
    poster.upload(hmm.dbfile.path, poster.upload_db_post(hmm.dbfile.path.name))
    logger.info(f"finished uploading {hmm.dbfile}")

    poster.db_post(DBName(name=hmm.dbfile.path.name))
    logger.info(f"finished posting {hmm.dbfile}")


class PresserManager:
    def __init__(self, poster: Poster, mqtt_host: str, mqtt_port: int):
        self.poster = poster
        self.requests: Queue[PressRequest] = Queue()
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
                    process_request(bg, self.poster, request)
                except Exception as exception:
                    logger.warning(f"pressing failed: {exception}")
                    job_update = JobUpdate.fail(request.job_id, str(exception))
                    bg.fire(partial(self.poster.job_patch, job_update))
                finally:
                    self.requests.task_done()
        client.loop_stop()

    def stop(self):
        self.requests.shutdown()
