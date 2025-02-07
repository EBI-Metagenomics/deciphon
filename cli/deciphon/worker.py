import os
import shutil
import signal
import sys
import uuid
from contextlib import contextmanager
from enum import Enum
from multiprocessing import Process
from pathlib import Path

import paho.mqtt.client as paho
from deciphon_core.sequence import Sequence
from deciphon_poster.poster import Poster
from deciphon_poster.schema import JobUpdate
from deciphon_schema import (
    DBFile,
    DBName,
    HMMFile,
    NewSnapFile,
    PressRequest,
    ScanRequest,
)
from deciphon_worker import launch_scanner, press
from loguru import logger
from paho.mqtt.client import Client, MQTTMessage

from deciphon.download import download
from deciphon.queue import ShuttableQueue, queue_loop

info = logger.info


class WorkType(str, Enum):
    scan = "scan"
    press = "press"


@contextmanager
def atomic_file_creation(path: Path):
    hex = str(uuid.uuid4().hex)[:16]
    tmp = path.with_suffix(f".{hex}tmp{path.suffix}")
    try:
        yield tmp
        shutil.move(tmp, path)
    finally:
        tmp.unlink(missing_ok=True)


class LogLevel(str, Enum):
    debug = "debug"
    info = "info"
    warning = "warning"
    error = "error"
    critical = "critical"

    # Enum of Python3.10 returns a different string representation.
    # Make it return the same as in Python3.11
    def __str__(self):
        return str(self.value)


def setup_logger(log_level: LogLevel):
    logger.remove()
    logger.add(sys.stderr, level=log_level.value.upper())


class ScanProcess:
    def __init__(
        self, poster: Poster, dbname: DBName, multi_hits: bool, hmmer3_compat: bool
    ):
        self._queue: ShuttableQueue[ScanRequest] = ShuttableQueue()
        self._poster = poster

        self._dbname = dbname
        self._multi_hits = multi_hits
        self._hmmer3_compat = hmmer3_compat
        self._process = Process(target=self._run)
        self._process.start()

    def _run(self):
        signal.signal(signal.SIGINT, signal.SIG_IGN)

        hmmpath = Path(self._dbname.hmmname.name)
        dbpath = Path(self._dbname.name)

        if not hmmpath.exists():
            with atomic_file_creation(hmmpath) as t:
                url = self._poster.download_hmm_url(self._dbname.hmmname.name)
                info(f"Downloading <{url}>")
                download(url, t)

        if not dbpath.exists():
            with atomic_file_creation(dbpath) as t:
                url = self._poster.download_db_url(self._dbname.name)
                info(f"Downloading <{url}>")
                download(url, t)

        dbfile = DBFile(path=dbpath)
        future = launch_scanner(
            dbfile, self._multi_hits, self._hmmer3_compat, cache=True
        )

        with future.result() as scanner:
            for x in queue_loop(self._queue):
                snappath: Path | None = None
                try:
                    info(f"Consuming <{x}> request...")

                    hex = str(uuid.uuid4().hex)[:8]
                    snap = NewSnapFile(path=Path(f"snapfile_scan_id{x.id}_{hex}.dcs"))
                    sequences = [Sequence(i.id, i.name, i.data) for i in x.seqs]

                    task = scanner.put(snap, sequences)
                    for i in task.as_progress():
                        info(f"Progress {i}% on scan_id <{x.id}>...")
                        self._poster.job_patch(JobUpdate.run(x.job_id, i))

                    info(f"Finished scanning scan_id <{x.id}>")

                    snappath = task.result().path

                    self._poster.snap_post(x.id, snappath)
                    info(f"Finished posting {snappath}")

                except Exception as exception:
                    fail = JobUpdate.fail(x.job_id, str(exception))
                    info(f"Failed to process {x}: {exception}")
                    self._poster.job_patch(fail)
                finally:
                    if snappath is not None:
                        snappath.unlink(missing_ok=True)
        info("Exitting scan process...")

    def add(self, x: ScanRequest):
        self._queue.put(x)

    def shutdown(self, immediate=False):
        self._queue.put("shutdown")
        if immediate:
            self._process.kill()

    def join(self):
        self._process.join()


class PressProcess:
    def __init__(self, poster: Poster):
        self._queue: ShuttableQueue[PressRequest] = ShuttableQueue()
        self._poster = poster
        self._process = Process(target=self._run)
        self._process.start()

    def _run(self):
        signal.signal(signal.SIGINT, signal.SIG_IGN)
        for x in queue_loop(self._queue):
            hmmpath: Path | None = None
            dbfile: DBFile | None = None
            try:
                info(f"Consuming <{x}> request...")
                url = self._poster.download_hmm_url(x.hmm.name)

                hex = str(uuid.uuid4().hex)[:8]
                hmmpath = Path(
                    f"gencode{x.gencode}_epsilon{x.epsilon}_{hex}_{x.hmm.name}"
                )

                info(f"Downloading <{url}>")
                download(url, hmmpath)
                hmmfile = HMMFile(path=hmmpath)

                info(f"Pressing <{hmmfile.path}>...")
                task = press(hmmfile, x.gencode, x.epsilon)

                for i in task.as_progress():
                    info(f"Progress {i}% on <{hmmfile.path}>...")
                    self._poster.job_patch(JobUpdate.run(x.job_id, i))

                dbfile = task.result()
                info(f"Finished creating <{dbfile.path}>")

                os.chmod(dbfile.path, 0o640)

                info(f"Uploading <{dbfile.path}>...")
                post = self._poster.upload_db_post(x.db.name)

                self._poster.upload(dbfile.path, post, name=x.db.name)
                info(f"Finished uploading <{dbfile.path}>")

                self._poster.db_post(x.db)
                info(f"Finished posting {x.db}")

            except Exception as exception:
                fail = JobUpdate.fail(x.job_id, str(exception))
                info(f"Failed to process {x}: {exception}")
                self._poster.job_patch(fail)
            finally:
                if hmmpath is not None:
                    hmmpath.unlink(missing_ok=True)
                if dbfile is not None:
                    dbfile.path.unlink(missing_ok=True)
        info("Exitting press process...")

    def add(self, x: PressRequest):
        self._queue.put(x)

    def shutdown(self, immediate=False):
        self._queue.put("shutdown")
        if immediate:
            self._process.kill()

    def join(self):
        self._process.join()


def scan_request_key(x: ScanRequest):
    return f"{x.db.name}-{x.multi_hits}-{x.hmmer3_compat}"


class ScanConsumer:
    def __init__(self, poster: Poster):
        self._poster = poster
        self._processes: dict[str, ScanProcess] = {}

    def add(self, payload: bytes):
        r = ScanRequest.model_validate_json(payload)

        info(f"Queuing scan request: {r}")
        key = scan_request_key(r)

        if key not in self._processes:
            self._processes[key] = ScanProcess(
                self._poster, r.db, r.multi_hits, r.hmmer3_compat
            )

        self._processes[key].add(r)

    def topic(self):
        return "/deciphon.org/scan"

    def shutdown(self, immediate=False):
        for x in self._processes.values():
            x.shutdown(immediate=immediate)

    def join(self):
        for x in self._processes.values():
            x.join()


class PressConsumer:
    def __init__(self, poster: Poster):
        self._process = PressProcess(poster)

    def add(self, payload: bytes):
        r = PressRequest.model_validate_json(payload)
        info(f"Queuing press request: {r}")
        self._process.add(r)

    def topic(self):
        return "/deciphon.org/press"

    def shutdown(self, immediate=False):
        self._process.shutdown(immediate=immediate)

    def join(self):
        self._process.join()


class Worker:
    def __init__(
        self,
        work: WorkType,
        poster: Poster,
        mqtt_host: str,
        mqtt_port: int,
    ):
        if work == WorkType.scan:
            self._consumer = ScanConsumer(poster)
        elif work == WorkType.press:
            self._consumer = PressConsumer(poster)

        mqtt_host = mqtt_host
        mqtt_port = mqtt_port

        self._client = paho.Client(paho.CallbackAPIVersion.VERSION2)  # type: ignore
        self._client.enable_logger()
        self._client.user_data_set(self._consumer)
        self._client.connect(mqtt_host, mqtt_port)

        def on_connect(
            client: Client,
            consumer: ScanConsumer | PressConsumer,
            flags,
            reason_code,
            properties,
        ):
            del flags
            del properties
            info(f"Connected with result code <{reason_code}>")
            client.subscribe(consumer.topic())
            info(f"Subscribed to <{consumer.topic()}>")

        def on_message(_, consumer: ScanConsumer | PressConsumer, msg: MQTTMessage):
            assert isinstance(msg.payload, bytes)
            info(f"Received: {msg.payload}")
            consumer.add(msg.payload)

        self._client.on_connect = on_connect
        self._client.on_message = on_message

    def loop_forever(self):
        self._client.loop_forever()

    def shutdown(self, immediate=False):
        if immediate:
            info("Forceful shutdown requested")
        else:
            info("Graceful shutdown requested")
        self._client.disconnect()
        self._client.loop_stop()
        self._consumer.shutdown(immediate=immediate)
        self._consumer.join()
