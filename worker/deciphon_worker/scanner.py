from __future__ import annotations

from functools import partial
from pathlib import Path
from queue import Queue

from deciphon_core.schema import HMMFile, HMMName
from deciphon_poster.poster import Poster
from deciphon_poster.schema import JobUpdate
from loguru import logger
from paho.mqtt.client import CallbackAPIVersion, Client

from deciphon_worker.background import Background
from deciphon_worker.download import download
from deciphon_worker.files import atomic_file_creation
from deciphon_worker.models import ScanRequest
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


def scanner_loop(poster: Poster, mqtt_host: str, mqtt_port: int):
    requests: Queue[ScanRequest] = Queue()
    scans: dict[int, ScanThread] = dict()

    logger.info(f"connecting to MQTT server (host={mqtt_host}, port={mqtt_port})")
    client = Client(CallbackAPIVersion.VERSION2, userdata=requests)
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect(mqtt_host, mqtt_port)

    client.loop_start()
    with Background() as bg:
        while True:
            request = requests.get()
            try:
                process_request(scans, bg, poster, request)
            except Exception as exception:
                logger.warning(f"scanning failed: {exception}")
                job_update = JobUpdate.fail(request.job_id, str(exception))
                bg.fire(partial(poster.job_patch, job_update))
            finally:
                requests.task_done()
    client.loop_stop()
